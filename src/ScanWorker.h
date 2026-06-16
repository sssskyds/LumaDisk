#ifndef LUMADISK_SCANWORKER_H
#define LUMADISK_SCANWORKER_H
#include "AppTypes.h"
#include <QThread>
#include <QHash>
#include <QElapsedTimer>
#include <atomic>
#include <QColor>

class ScanWorker : public QThread {
Q_OBJECT
public:
ScanWorker(QString root, bool includeHidden, QObject* parent=nullptr);
void cancel();
signals:
void progress(const QString& path, qint64 files, quint64 bytes);
void finishedResult(const ScanResult& result);
protected:
void run() override;
private:
quint64 scanDir(const QString& path, FolderRow& row);
void addFile(const QFileInfo& info);
QString categoryFor(const QString& suffix, QColor* color=nullptr) const;
QString bytes(quint64 n) const;
QString m_root; bool m_hidden=false; std::atomic_bool m_cancel{false};
ScanResult m_result; QHash<QString,TypeStat> m_types; QElapsedTimer m_timer; qint64 m_last=0;
};
#endif