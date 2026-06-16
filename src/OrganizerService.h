#ifndef LUMADISK_ORGANIZERSERVICE_H
#define LUMADISK_ORGANIZERSERVICE_H
#include "AppTypes.h"
#include <QVector>
#include <QDir>
#include <QFileInfo>

class OrganizerService {
public:
QVector<OrganizeAction> preview(const QString& sourceDir, bool recursive, bool includeHidden, bool preserveSubfolders, OrganizeRule rule) const;
OrganizeSummary apply(const QVector<OrganizeAction>& actions, ConflictMode mode) const;
private:
QString folderFor(const QFileInfo& info, const QDir& root, bool preserve, OrganizeRule rule) const;
QString sanitize(QString s) const;
QString autoName(const QString& path) const;
QString ruleName(OrganizeRule rule) const;
QString category(const QString& suffix) const;
};
#endif