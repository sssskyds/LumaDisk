#ifndef LUMADISK_APPTYPES_H
#define LUMADISK_APPTYPES_H
#include <QString>
#include <QVector>
#include <QColor>
#include <QDateTime>
#include <QStringList>
#include <QMetaType>

struct FileRow { QString name, path, extension, category; QColor color; quint64 size=0; QDateTime modified; };
struct FolderRow { QString name, path; quint64 size=0; qint64 files=0; qint64 folders=0; };
struct TypeStat { QString category; QColor color; quint64 size=0; qint64 count=0; };
struct ScanResult { QString rootPath; quint64 total=0; qint64 files=0; qint64 folders=0; qint64 elapsedMs=0; bool cancelled=false; QVector<FileRow> largest; QVector<FolderRow> folderRows; QVector<TypeStat> types; QStringList errors; };

struct OrganizeAction { QString source, destination, rule, status; quint64 size=0; bool conflict=false; };
struct OrganizeSummary { int moved=0, skipped=0, failed=0; QStringList errors; };

enum class OrganizeRule { Category, Extension, YearMonth, SizeTier };
enum class ConflictMode { AutoRename, Skip, Overwrite };

Q_DECLARE_METATYPE(ScanResult)
#endif