#ifndef LUMADISK_MAINWINDOW_H
#define LUMADISK_MAINWINDOW_H
#include "AppTypes.h"
#include "ScanWorker.h"
#include "OrganizerService.h"
#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QProgressBar>
#include <QLabel>
#include <QLineEdit>
#include <QTabWidget>
#include <QTableWidget>

class MainWindow: public QMainWindow{
Q_OBJECT
public: explicit MainWindow(QWidget* parent=nullptr); ~MainWindow() override;
private slots: void chooseFolder(); void startScan(); void cancelScan(); void onProgress(const QString&,qint64,quint64); void onDone(const ScanResult&); void filterFiles(const QString&); void previewOrganize(); void applyOrganize();
private: void build(); void loadDrives(); void setBusy(bool); void populate(const ScanResult&); void fillFiles(const QVector<FileRow>&); void fillFolders(const QVector<FolderRow>&); void fillTypes(const QVector<TypeStat>&); QString bytes(quint64) const; OrganizeRule selectedRule() const; ConflictMode selectedConflict() const; void fillActions(const QVector<OrganizeAction>&);
QComboBox *drive=nullptr,*orgRule=nullptr,*orgConflict=nullptr; QPushButton *choose=nullptr,*scan=nullptr,*cancel=nullptr,*orgPreview=nullptr,*orgApply=nullptr; QCheckBox *hidden=nullptr,*orgRecursive=nullptr,*orgPreserve=nullptr,*orgHidden=nullptr; QProgressBar* progress=nullptr; QLabel *status=nullptr,*summary=nullptr; QLineEdit *filter=nullptr,*orgSource=nullptr; QTabWidget* tabs=nullptr; QTableWidget *folders=nullptr,*files=nullptr,*types=nullptr,*organizer=nullptr,*notes=nullptr; ScanWorker* worker=nullptr; ScanResult current; QVector<FileRow> allFiles; QVector<OrganizeAction> actions; OrganizerService orgService;
};
#endif