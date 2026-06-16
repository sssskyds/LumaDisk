#include "MainWindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QFile>
#include <QStyle>
int main(int argc,char*argv[]){
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling); QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
QApplication app(argc,argv); QCoreApplication::setApplicationName("LumaDisk"); QCoreApplication::setOrganizationName("LumaDisk"); if(QStyle*s=QStyleFactory::create("Fusion"))app.setStyle(s); QFile f(":/styles/luma_dark.qss"); if(f.open(QIODevice::ReadOnly|QIODevice::Text)) app.setStyleSheet(QString::fromUtf8(f.readAll())); MainWindow w; w.show(); return app.exec(); }