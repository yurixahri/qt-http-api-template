#ifndef WEBSOCKETHELPER_H
#define WEBSOCKETHELPER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QCryptographicHash>
#include <QWebSocket>
#include <QJsonObject>
#include <QFile>
#include "utils/stringUtils.h"
#include "utils/fs.h"

struct uploadFile {
    QFile file;
    qint64 expectedSize = 0;
    qint64 receivedSize = 0;
};

extern QHash<QWebSocket*, uploadFile*> uploads;

void emitEvent(QWebSocket *ws, const uint8_t &event, const QJsonValue &data = NULL) ;

void onEvent(QWebSocket *ws, const uint8_t &event, const QJsonValue &data);

void uploadStart(QWebSocket *ws, const QJsonValue &data);
void removeUpload(QWebSocket *ws);
QString makeUniqueName(const QString &absolutePath, const QString &originalName);

#endif // WEBSOCKETHELPER_H
