#include "ScanWorker.h"
#include <QDir>
#include <QFileInfo>
#include <QDirIterator>

ScanWorker::ScanWorker(QString root, bool includeHidden, QObject* parent):QThread(parent),m_root(std::move(root)),m_hidden(includeHidden){}
void ScanWorker::cancel(){m_cancel.store(true);}

void ScanWorker::run(){
qRegisterMetaType<ScanResult>("ScanResult"); m_timer.start(); m_result.rootPath=QDir::cleanPath(m_root);
QFileInfo rootInfo(m_root); if(!rootInfo.exists()||!rootInfo.isDir()){m_result.errors<<"Invalid source: "+m_root; emit finishedResult(m_result); return;}
FolderRow rootRow; rootRow.name=rootInfo.fileName().isEmpty()?rootInfo.absoluteFilePath():rootInfo.fileName(); rootRow.path=rootInfo.absoluteFilePath();
m_result.total=scanDir(rootInfo.absoluteFilePath(), rootRow); m_result.folderRows.push_front(rootRow);
for(auto it=m_types.begin(); it!=m_types.end(); ++it) m_result.types.push_back(it.value());
std::sort(m_result.largest.begin(),m_result.largest.end(),[](const FileRow&a,const FileRow&b){return a.size>b.size;});
std::sort(m_result.folderRows.begin(),m_result.folderRows.end(),[](const FolderRow&a,const FolderRow&b){return a.size>b.size;});
std::sort(m_result.types.begin(),m_result.types.end(),[](const TypeStat&a,const TypeStat&b){return a.size>b.size;});
if(m_result.largest.size()>1000) m_result.largest.resize(1000); m_result.cancelled=m_cancel.load(); m_result.elapsedMs=m_timer.elapsed(); emit finishedResult(m_result);}

quint64 ScanWorker::scanDir(const QString& path, FolderRow& row){
if(m_cancel.load()) return 0; QDir dir(path); QDir::Filters filters=QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot|QDir::System; if(m_hidden) filters|=QDir::Hidden;
QFileInfoList entries=dir.entryInfoList(filters,QDir::DirsFirst|QDir::Name); quint64 total=0;
for(const QFileInfo& info: entries){ if(m_cancel.load()) break; if(info.isSymLink()) continue; QString n=info.fileName(); if(n==".git"||n=="node_modules"||n==".venv"||n=="__pycache__") continue;
if(info.isDir()){ FolderRow child; child.name=info.fileName(); child.path=info.absoluteFilePath(); quint64 s=scanDir(child.path, child); child.size=s; row.folders += 1+child.folders; row.files += child.files; total+=s; m_result.folderRows.push_back(child); }
else if(info.isFile()){ addFile(info); row.files++; total += static_cast<quint64>(qMax(0,info.size())); }
if(m_timer.elapsed()-m_last>120){m_last=m_timer.elapsed(); emit progress(info.absoluteFilePath(),m_result.files,m_result.total);} }
row.size=total; return total;}

void ScanWorker::addFile(const QFileInfo& info){
QColor color; QString cat=categoryFor(info.suffix(),&color); quint64 size=static_cast<quint64>(qMax(0,info.size()));
FileRow f{info.fileName(),info.absoluteFilePath(),info.suffix().isEmpty()?"no extension":"."+info.suffix().toLower(),cat,color,size,info.lastModified()};
m_result.files++; m_result.total+=size; m_result.largest.push_back(f); auto& t=m_types[cat]; if(t.category.isEmpty()){t.category=cat;t.color=color;} t.size+=size;t.count++;
if(m_result.largest.size()>1500){std::sort(m_result.largest.begin(),m_result.largest.end(),[](const FileRow&a,const FileRow&b){return a.size>b.size;});m_result.largest.resize(1000);}}

QString ScanWorker::categoryFor(const QString& s, QColor* color) const{ QString e=s.toLower();
auto ret=[&](QString c, QString col){ if(color)*color=QColor(col); return c;};
static QSet<QString> img={"jpg","jpeg","png","gif","bmp","svg","webp","heic"}, vid={"mp4","mkv","mov","avi","webm"}, aud={"mp3","wav","flac","aac","ogg"}, doc={"pdf","doc","docx","xls","xlsx","ppt","pptx","txt","csv"}, code={"cpp","c","h","hpp","py","js","ts","java","html","css","json","xml","md"}, arc={"zip","rar","7z","tar","gz","iso"}, exe={"exe","msi","dll","sys"};
if(img.contains(e))return ret("Images","#38bdf8"); if(vid.contains(e))return ret("Videos","#a78bfa"); if(aud.contains(e))return ret("Audio","#f472b6"); if(doc.contains(e))return ret("Documents","#facc15"); if(code.contains(e))return ret("Code","#34d399"); if(arc.contains(e))return ret("Archives","#fb923c"); if(exe.contains(e))return ret("Executables","#fb7185"); return ret("Other","#94a3b8");}