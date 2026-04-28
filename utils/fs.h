#ifndef FS_H
#define FS_H


#include <QDir>
#include <QDirIterator>
#include <QStringList>
#include <QString>
#include <QCryptographicHash>
#include <QSettings>
#include <QCollator>
#include <QMimeType>
#include <QMimeDatabase>
#include <QStorageInfo>
#include <QFuture>

// #include <optional>
// #include "utils/stringUtils.h"
// #include "utils/config.h"

// const QStringList ext_image = {"jpg", "png", "jpeg", "jfif", "tiff", "avif", "gif", "webp"};
// const QStringList ext_music = {"mp3", "flac", "ogg", "aiff", "wav", "m4a", "aac", "alac", "wma", "opus"};

//uint _count = 0;

// struct songMetadata{
//     QString folder;
//     QString file;
//     QString title;
//     QString album;
//     QString artist;
//     QString hash;
//     QString thumbnail;
//     QString cover;

//     songMetadata(){}

// };

inline QStringList getMountedVolumes() {
    QStringList list;
    QList<QStorageInfo> volumes = QStorageInfo::mountedVolumes();

    for (const QStorageInfo &storage : volumes) {
        if (storage.isReady() && !storage.isReadOnly()) {
            list.append(storage.rootPath());
        }
    }
    return list;
}

// deprecated
// inline QStringList getDrives(){
//     QFileInfoList drives = QDir::drives();
//     QStringList list;
//     for (auto &drive : drives)
//         list.append(drive.absolutePath());
//     return list;
// }

inline bool isExist(const QString &path){
    QFileInfo info(path);
    return info.exists();
}

inline bool isFile(const QString &path){
    QFileInfo info(path);
    return info.isFile();
}

inline QMimeType getMimeType(const QString &path){
    QFileInfo info(path);
    QMimeDatabase mimeDatabase;
    return mimeDatabase.mimeTypeForFile(info);
}


inline QStringList getDirectories(const QString &path){
    QDir dir(path);

    QStringList list = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    QCollator collator;
    collator.setNumericMode(true);
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    std::sort(list.begin(), list.end(), [&](const QString &a, const QString &b) {
        return collator.compare(a, b) < 0;
    });
    return list;
}

inline QStringList getFiles(const QString &path){
    QDir dir(path);

    QStringList list = dir.entryList(QDir::Files);
    QCollator collator;
    collator.setNumericMode(true);
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    std::sort(list.begin(), list.end(), [&](const QString &a, const QString &b) {
        return collator.compare(a, b) < 0;
    });
    return list;
}

inline QString getExt(QString &string){
    QString ext = string.split(".").back();
    return ext;
}

inline bool removeFile(QString path){
    return QFile::remove(path);
}

inline bool removeDirectory(QString path){
    QDir dir(path);
    return dir.removeRecursively();
}

inline bool makeDirectory(QString path){
    QDir dir;
    if (!dir.exists(path)) {
        if (dir.mkpath(path)) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

inline qint64 getDirectorySize(const QString &dirPath)
{
    qint64 total = 0;
    QDir dir(dirPath);

    // Only include files directly in this directory (no subfolders)
    QFileInfoList files = dir.entryInfoList(
        QDir::Files | QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot);

    for (const QFileInfo &fi : files) {
        if (fi.exists() && fi.isFile())
            total += fi.size();
    }

    return total;
}

#endif // FS_H
