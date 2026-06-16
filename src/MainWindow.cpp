#include "MainWindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QStorageInfo>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QSplitter>
#include <QSizePolicy>
#include <QFrame>
#include <QDir>

MainWindow::MainWindow(QWidget* parent):QMainWindow(parent){ build(); loadDrives(); }
MainWindow::~MainWindow(){ if(worker){worker->cancel();worker->wait();} }

void MainWindow::build(){
// Central widget + root layout
QWidget* root=new QWidget; setCentralWidget(root); QVBoxLayout* vl=new QVBoxLayout(root); vl->setContentsMargins(16,16,16,16); vl->setSpacing(10);
// Header
QLabel* brand=new QLabel("LumaDisk"); brand->setObjectName("Brand"); vl->addWidget(brand);
// Scan controls
QHBoxLayout* hl=new QHBoxLayout; drive=new QComboBox; drive->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed); choose=new QPushButton("Choose"); scan=new QPushButton("Scan"); scan->setObjectName("Primary"); cancel=new QPushButton("Cancel"); cancel->setObjectName("Danger"); hidden=new QCheckBox("Hidden"); hl->addWidget(drive); hl->addWidget(choose); hl->addWidget(scan); hl->addWidget(cancel); hl->addWidget(hidden); vl->addLayout(hl);
// Progress
progress=new QProgressBar; progress->setRange(0,0); progress->setVisible(false); vl->addWidget(progress);
// Status
status=new QLabel("Ready."); summary=new QLabel; vl->addWidget(status); vl->addWidget(summary);
// Filter
filter=new QLineEdit; filter->setPlaceholderText("Filter files..."); vl->addWidget(filter);
connect(filter,&QLineEdit::textChanged,this,&MainWindow::filterFiles);
// Tabs
tabs=new QTabWidget; vl->addWidget(tabs,1);
// Folder Weight tab
folders=new QTableWidget(0,4); folders->setHorizontalHeaderLabels({"Folder","Size","Files","Subfolders"}); folders->horizontalHeader()->setStretchLastSection(true); folders->setAlternatingRowColors(true); tabs->addTab(folders,"Folder Weight");
// Largest Files tab
files=new QTableWidget(0,4); files->setHorizontalHeaderLabels({"Name","Path","Size","Type"}); files->horizontalHeader()->setStretchLastSection(true); files->setAlternatingRowColors(true); tabs->addTab(files,"Largest Files");
// File Types tab
types=new QTableWidget(0,4); types->setHorizontalHeaderLabels({"Category","Size","Count","Share"}); types->horizontalHeader()->setStretchLastSection(true); types->setAlternatingRowColors(true); tabs->addTab(types,"File Types");
// Organizer tab
QWidget* orgTab=new QWidget; QVBoxLayout* ovl=new QVBoxLayout(orgTab); tabs->addTab(orgTab,"Organizer");
QHBoxLayout* osrc=new QHBoxLayout; orgSource=new QLineEdit; orgSource->setPlaceholderText("Source folder..."); QPushButton* orgChoose=new QPushButton("Browse"); osrc->addWidget(orgSource); osrc->addWidget(orgChoose); ovl->addLayout(osrc);
QFormLayout* ofrm=new QFormLayout; orgRule=new QComboBox; orgRule->addItems({"File Type Category","Extension","Modified Year/Month","Size Tier"}); orgConflict=new QComboBox; orgConflict->addItems({"Auto-rename","Skip","Overwrite"}); orgRecursive=new QCheckBox("Include files inside subfolders"); orgRecursive->setChecked(true); orgPreserve=new QCheckBox("Preserve original subfolder structure"); orgHidden=new QCheckBox("Include hidden/system files"); ofrm->addRow("Organize by:",orgRule); ofrm->addRow("Conflict mode:",orgConflict); ofrm->addRow("",orgRecursive); ofrm->addRow("",orgPreserve); ofrm->addRow("",orgHidden); ovl->addLayout(ofrm);
QHBoxLayout* obt=new QHBoxLayout; orgPreview=new QPushButton("Preview"); orgPreview->setObjectName("Primary"); orgApply=new QPushButton("Apply Organization"); orgApply->setEnabled(false); obt->addWidget(orgPreview); obt->addWidget(orgApply); ovl->addLayout(obt);
organizer=new QTableWidget(0,5); organizer->setHorizontalHeaderLabels({"Source","Destination","Rule","Size","Status"}); organizer->horizontalHeader()->setStretchLastSection(true); organizer->setAlternatingRowColors(true); ovl->addWidget(organizer,1);
// Connect
connect(choose,&QPushButton::clicked,this,&MainWindow::chooseFolder);
connect(scan,&QPushButton::clicked,this,&MainWindow::startScan);
connect(cancel,&QPushButton::clicked,this,&MainWindow::cancelScan);
connect(orgChoose,&QPushButton::clicked,this,[this](){QString d=QFileDialog::getExistingDirectory(this,"Select source folder"); if(!d.isEmpty()) orgSource->setText(d);});
connect(orgPreview,&QPushButton::clicked,this,&MainWindow::previewOrganize);
connect(orgApply,&QPushButton::clicked,this,&MainWindow::applyOrganize);
setWindowTitle("LumaDisk"); resize(1100,700); setBusy(false);}

void MainWindow::loadDrives(){ drive->clear(); for(const QStorageInfo& s:QStorageInfo::mountedVolumes()) if(s.isValid()&&s.isReady()) drive->addItem(s.rootPath());}
void MainWindow::chooseFolder(){ QString d=QFileDialog::getExistingDirectory(this,"Select Folder",drive->currentText()); if(!d.isEmpty()) drive->setCurrentText(d);}
void MainWindow::setBusy(bool b){ scan->setEnabled(!b); cancel->setEnabled(b); progress->setVisible(b);}
void MainWindow::startScan(){ if(worker){worker->cancel();worker->wait();delete worker;worker=nullptr;} setBusy(true); status->setText("Scanning..."); worker=new ScanWorker(drive->currentText(),hidden->isChecked(),this); connect(worker,&ScanWorker::progress,this,&MainWindow::onProgress); connect(worker,&ScanWorker::finishedResult,this,&MainWindow::onDone); worker->start();}
void MainWindow::cancelScan(){ if(worker) worker->cancel();}
void MainWindow::onProgress(const QString& p,qint64 f,quint64){status->setText(QString("Scanning: %1 files — %2").arg(f).arg(p.right(50)));}
void MainWindow::onDone(const ScanResult& r){ current=r; allFiles=r.largest; setBusy(false); populate(r); status->setText(QString("Done: %1 files, %2 folders in %3 ms%4").arg(r.files).arg(r.folders).arg(r.elapsedMs).arg(r.cancelled?" (cancelled)":"")); summary->setText(QString("Total: %1").arg(bytes(r.total)));}
void MainWindow::populate(const ScanResult& r){ fillFolders(r.folderRows); fillFiles(r.largest); fillTypes(r.types);}
void MainWindow::fillFolders(const QVector<FolderRow>&v){folders->setRowCount(0);for(const auto&f:v){int r=folders->rowCount();folders->insertRow(r);folders->setItem(r,0,new QTableWidgetItem(f.name));folders->setItem(r,1,new QTableWidgetItem(bytes(f.size)));folders->setItem(r,2,new QTableWidgetItem(QString::number(f.files)));folders->setItem(r,3,new QTableWidgetItem(QString::number(f.folders)));}}
void MainWindow::fillFiles(const QVector<FileRow>&v){files->setRowCount(0);for(const auto&f:v){int r=files->rowCount();files->insertRow(r);files->setItem(r,0,new QTableWidgetItem(f.name));files->setItem(r,1,new QTableWidgetItem(f.path));files->setItem(r,2,new QTableWidgetItem(bytes(f.size)));files->setItem(r,3,new QTableWidgetItem(f.category));}}
void MainWindow::fillTypes(const QVector<TypeStat>&v){types->setRowCount(0);for(const auto&t:v){int r=types->rowCount();types->insertRow(r);types->setItem(r,0,new QTableWidgetItem(t.category));types->setItem(r,1,new QTableWidgetItem(bytes(t.size)));types->setItem(r,2,new QTableWidgetItem(QString::number(t.count)));types->setItem(r,3,new QTableWidgetItem(current.total?QString::number(double(t.size)*100.0/double(current.total),'f',1)+"%":"0%"));}}
void MainWindow::filterFiles(const QString&s){if(s.trimmed().isEmpty()){fillFiles(allFiles);return;}QVector<FileRow> out;QString n=s.toLower();for(const auto&f:allFiles)if(f.name.toLower().contains(n)||f.path.toLower().contains(n)||f.category.toLower().contains(n)||f.extension.toLower().contains(n))out<<f;fillFiles(out);}
QString MainWindow::bytes(quint64 n) const{if(n<1024)return QString::number(n)+" B";if(n<1024*1024)return QString::number(n/1024.0,'f',1)+" KB";if(n<1024ULL*1024*1024)return QString::number(n/1024.0/1024,'f',1)+" MB";return QString::number(n/1024.0/1024/1024,'f',2)+" GB";}
OrganizeRule MainWindow::selectedRule()const{switch(orgRule->currentIndex()){case 1:return OrganizeRule::Extension;case 2:return OrganizeRule::YearMonth;case 3:return OrganizeRule::SizeTier;default:return OrganizeRule::Category;}}
ConflictMode MainWindow::selectedConflict()const{switch(orgConflict->currentIndex()){case 1:return ConflictMode::Skip;case 2:return ConflictMode::Overwrite;default:return ConflictMode::AutoRename;}}
void MainWindow::previewOrganize(){actions=orgService.preview(orgSource->text(),orgRecursive->isChecked(),orgHidden->isChecked(),orgPreserve->isChecked(),selectedRule());fillActions(actions);orgApply->setEnabled(!actions.isEmpty());status->setText(QString("Organizer preview: %1 files. Output: _Luma Organized").arg(actions.size()));}
void MainWindow::fillActions(const QVector<OrganizeAction>&a){organizer->setRowCount(0);for(const auto&x:a){int r=organizer->rowCount();organizer->insertRow(r);organizer->setItem(r,0,new QTableWidgetItem(x.source));organizer->setItem(r,1,new QTableWidgetItem(x.destination));organizer->setItem(r,2,new QTableWidgetItem(x.rule));organizer->setItem(r,3,new QTableWidgetItem(bytes(x.size)));organizer->setItem(r,4,new QTableWidgetItem(x.status));}}
void MainWindow::applyOrganize(){if(actions.isEmpty())return;if(QMessageBox::question(this,"Apply Organization",QString("Move %1 files into _Luma Organized?").arg(actions.size()))!=QMessageBox::Yes)return;auto s=orgService.apply(actions,selectedConflict());status->setText(QString("Organizer done: moved %1, skipped %2, failed %3").arg(s.moved).arg(s.skipped).arg(s.failed));if(!s.errors.isEmpty())QMessageBox::warning(this,"Some files failed",s.errors.mid(0,10).join("\n"));actions.clear();organizer->setRowCount(0);orgApply->setEnabled(false);}