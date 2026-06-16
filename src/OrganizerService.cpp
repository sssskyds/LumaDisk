#include "OrganizerService.h"
#include <QDirIterator>
#include <QFile>
#include <QDir>
#include <QFileInfo>

QVector<OrganizeAction> OrganizerService::preview(const QString& sourceDir, bool recursive, bool includeHidden, bool preserveSubfolders, OrganizeRule rule) const{
QVector<OrganizeAction> out; QDir root(sourceDir); if(!root.exists()) return out; QString org=QDir::cleanPath(root.filePath("_Luma Organized"));
QDir::Filters filters=QDir::Files|QDir::NoDotAndDotDot|QDir::System; if(includeHidden) filters|=QDir::Hidden; QDirIterator::IteratorFlags flags=recursive?QDirIterator::Subdirectories:QDirIterator::NoIteratorFlags;
QDirIterator it(root.absolutePath(), filters, flags); while(it.hasNext()){ it.next(); QFileInfo info=it.fileInfo(); if(!info.isFile()||info.isSymLink()) continue; QString src=QDir::cleanPath(info.absoluteFilePath()); if(src==org||src.startsWith(org+"/")) continue;
QString dest=QDir::cleanPath(QDir(folderFor(info,root,preserveSubfolders,rule)).filePath(info.fileName())); if(dest==src) continue; bool conflict=QFileInfo::exists(dest); out.push_back({src,dest,ruleName(rule),conflict?"Conflict":"Ready",static_cast<quint64>(qMax(0,info.size())),conflict});}
return out;}

OrganizeSummary OrganizerService::apply(const QVector<OrganizeAction>& actions, ConflictMode mode) const{ OrganizeSummary s; for(const auto&a:actions){ QString dest=a.destination; if(QFileInfo::exists(dest)){ if(mode==ConflictMode::Skip){s.skipped++;continue;} if(mode==ConflictMode::AutoRename) dest=autoName(dest); else if(!QFile::remove(dest)){s.failed++;s.errors<<"Cannot overwrite: "+dest;continue;}}
QDir().mkpath(QFileInfo(dest).absolutePath()); if(QFile::rename(a.source,dest) || (QFile::copy(a.source,dest)&&QFile::remove(a.source))) s.moved++; else {s.failed++;s.errors<<"Failed: "+a.source;}} return s;}

QString OrganizerService::folderFor(const QFileInfo& info,const QDir& root,bool preserve,OrganizeRule rule) const{ QString g; switch(rule){case OrganizeRule::Category:g=category(info.suffix());break;case OrganizeRule::Extension:g=info.suffix().isEmpty()?"no extension":info.suffix().toLower();break;case OrganizeRule::YearMonth:g=info.lastModified().isValid()?info.lastModified().toString("yyyy/MM - MMMM"):"Unknown Date";break;case OrganizeRule::SizeTier:{quint64 b=static_cast<quint64>(qMax(0,info.size())); g=b<1024?"Tiny under 1 KB":b<1024ULL*1024?"Small 1 KB to 1 MB":b<100ULL*1024*1024?"Medium 1 MB to 100 MB":b<1024ULL*1024*1024?"Large 100 MB to 1 GB":"Huge over 1 GB";break;}}
QString f=root.filePath("_Luma Organized/"+sanitize(g)); if(preserve){QString rel=root.relativeFilePath(info.absolutePath()); if(!rel.isEmpty()&&rel!=".") f=QDir(f).filePath(rel);} return f;}
QString OrganizerService::sanitize(QString s) const{for(QChar c:QString("\\:*?\"<>|"))s.replace(c,'_');return s.trimmed().isEmpty()?"Other":s.trimmed();}
QString OrganizerService::autoName(const QString& path) const{QFileInfo i(path);for(int n=2;n<10000;n++){QString c=i.suffix().isEmpty()?QString("%1/%2 (%3)").arg(i.absolutePath(),i.completeBaseName()).arg(n):QString("%1/%2 (%3).%4").arg(i.absolutePath(),i.completeBaseName()).arg(n).arg(i.suffix()); if(!QFileInfo::exists(c))return c;}return path;}
QString OrganizerService::ruleName(OrganizeRule r) const{switch(r){case OrganizeRule::Category:return "Category";case OrganizeRule::Extension:return "Extension";case OrganizeRule::YearMonth:return "Year/Month";case OrganizeRule::SizeTier:return "Size Tier";}return "Rule";}
QString OrganizerService::category(const QString& suffix) const{QString e=suffix.toLower(); static QSet<QString> img={"jpg","jpeg","png","gif","bmp","svg","webp"},vid={"mp4","mkv","mov","avi"},aud={"mp3","wav","flac"},doc={"pdf","doc","docx","txt","xls","xlsx","ppt","pptx"},arc={"zip","rar","7z","tar","gz"}; if(img.contains(e))return "Images";if(vid.contains(e))return "Videos";if(aud.contains(e))return "Audio";if(doc.contains(e))return "Documents";if(arc.contains(e))return "Archives";return "Other";}