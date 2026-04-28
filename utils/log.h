#ifndef LOG_H
#define LOG_H

#include <QFile>
#include <QDateTime>
#include <utils/stringUtils.h>

inline void writeLog(const QString &message) {
    QFile logFile("./log.txt");

    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        out << timestamp << ": " << message << "\n";

        logFile.close();
    } else {
        logError("Error opening log file for writing.");
    }
}

#endif // LOG_H
