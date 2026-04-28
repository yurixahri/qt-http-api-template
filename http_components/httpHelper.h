#ifndef HTTPHELPER_H
#define HTTPHELPER_H

#include <QHttpHeaders>
#include <QTcpServer>
#include <QHttpServer>
#include <QNetworkCookie>
#include <QMimeType>

#include <QJsonParseError>
#include <QJsonDocument>

#include <QFileInfo>
#include "utils/log.h"
#include "utils/fs.h"
#include <optional>

typedef struct {
    bool cors;
    bool cache;
} headerFlags;

#define createHeaders(...) \
_createHeaders((headerFlags){.cors = true, .cache = true})

QHttpHeaders _createHeaders(headerFlags flags){
    QHttpHeaders headers;
    if (flags.cors){
        headers.append("Access-Control-Allow-Origin", "*");
        headers.append("Access-Control-Allow-Methods", "GET, POST");
        headers.append("Access-Control-Allow-Headers", "Content-Type, *");
        headers.append("Cross-Origin-Resource-Policy", "cross-origin");
    }
    if (!flags.cache){
        headers.append("Cache-Control", "no-store, no-cache, must-revalidate");
        headers.append("Pragma", "no-cache");
    }
    return headers;
}

bool tcpStart(QTcpServer &tcpServer, QHttpServer &server, const uint16_t &port){
    if (!tcpServer.listen(QHostAddress::Any, port) || !server.bind(&tcpServer)) {
        logError("Server failed to listen on a port " + std::to_string(port));
        return false;
    }else{
        logSuccess("Server established successfully on port " + std::to_string(port));
        return true;
    }
}

bool tcpReload(QTcpServer &tcpServer, QHttpServer &server, const uint16_t &port){
    if (tcpServer.isListening()) {
        logWarning("Server is closing on port "+ std::to_string(tcpServer.serverPort()));
        tcpServer.close();
    }
    return tcpStart(tcpServer, server, port);
}

// QHttpServerResponse sendWithCors(QHttpServerResponse &res){
//     res.setHeaders(createHeaders());
// }

// std::optional<QHttpServerResponse> isBodyInvalid(QJsonParseError &err, QJsonDocument &doc) {
//     if (err.error != QJsonParseError::NoError) {
//         QHttpServerResponse response("Invalid JSON", QHttpServerResponse::StatusCode::BadRequest);
//         response.setHeaders(createHeaders(.cache = false));
//         return response;
//     }

//     if (!doc.isObject()) {
//         QHttpServerResponse response("Expected JSON object", QHttpServerResponse::StatusCode::BadRequest);
//         response.setHeaders(createHeaders(.cache = false));
//         return response;
//     }

//     return std::nullopt;
// }

std::optional<QHttpServerResponse> parseBody(QJsonDocument &doc, const QByteArray &raw){
    QJsonParseError err;
    doc = QJsonDocument::fromJson(raw, &err);

    if (err.error != QJsonParseError::NoError) {
        QHttpServerResponse response("Invalid JSON", QHttpServerResponse::StatusCode::BadRequest);
        response.setHeaders(createHeaders(.cache = false));
        return response;
    }

    if (!doc.isObject()) {
        QHttpServerResponse response("Expected JSON object", QHttpServerResponse::StatusCode::BadRequest);
        response.setHeaders(createHeaders(.cache = false));
        return response;
    }

    return std::nullopt;
}

std::optional<QHttpServerResponse> isContentTypeInvalid(const QHttpServerRequest &request, const std::string &type) {
    const auto contentType = request.value("Content-Type");
    if (!contentType.contains(type)) {
        QHttpServerResponse response("Unsupported Media Type", QHttpServerResponse::StatusCode::UnsupportedMediaType);
        response.setHeaders(createHeaders(.cache = false));
        return response;
    }

    return std::nullopt;
}



std::optional<QMap<QString, QString>> getCookies(const QHttpServerRequest &request){
    QByteArrayView cookieHeader = request.headers().value(QHttpHeaders::WellKnownHeader::Cookie);

    if (cookieHeader.isEmpty()) return std::nullopt;

    QMap<QString, QString> cookies;
    const QList<QByteArray> pairs = cookieHeader.toByteArray().split(';');

    for (const QByteArray &pair : pairs) {
        QByteArray trimmed = pair.trimmed();
        if (trimmed.isEmpty())
            continue;

        int eqPos = trimmed.indexOf('=');
        if (eqPos <= 0)
            continue; // skip invalid cookies like "Secure" or "HttpOnly"

        QString name = QString::fromUtf8(trimmed.left(eqPos).trimmed());
        QString value = QString::fromUtf8(trimmed.mid(eqPos + 1).trimmed());

        cookies.insert(name, value);
    }

    return cookies;
}

QHttpServerResponse sendStatus(QHttpServerResponse::StatusCode status){
    QHttpServerResponse response(status);
    response.setHeaders(createHeaders(.cache = false));
    return response;
}

QHttpServerResponse sendStatus(const QString message, const QHttpServerResponse::StatusCode status){
    QHttpServerResponse response(message, status);
    response.setHeaders(createHeaders(.cache = false));
    return response;
}

void serveFile(QString &path, const QHttpServerRequest &request, QHttpServerResponder &responder){
    QHttpHeaders headers = createHeaders(.cache = false);

    QFileInfo info(path);
    QFile *file = new QFile(path);
    if (!file->open(QIODevice::ReadOnly)) {
        delete file;
        responder.write("Cannot open file", "text/plain");
        responder.write(headers, QHttpServerResponder::StatusCode::InternalServerError);
        logError("can't open file");
        return;
    }

    qint64 fileSize = file->size();
    qint64 start = 0;
    qint64 end = fileSize - 1;

    auto rangeHeader = request.value("Range");
    if (!rangeHeader.isEmpty()) {
        static const QRegularExpression regex("bytes=(\\d+)-(\\d*)");
        QRegularExpressionMatch match = regex.match(rangeHeader);
        if (match.hasMatch()) {
            start = match.captured(1).toLongLong();
            if (!match.captured(2).isEmpty()) {
                end = match.captured(2).toLongLong();
            }
        }
    }

    if (start > fileSize - 1) {
        responder.write(QByteArray("File not found"), QByteArray("text/plain"), QHttpServerResponder::StatusCode::NotFound);
        file->close();
        delete file;
        responder.write(headers, QHttpServerResponder::StatusCode::InternalServerError);
        return;
    }

    end = qMin(end, fileSize - 1);
    //qint64 contentLength = end - start + 1;
    file->seek(start);
    //QByteArray data = file->read(contentLength);

    QMimeType mimeType = getMimeType(path);

    auto status = (start > 0)   ? QHttpServerResponder::StatusCode::PartialContent
                              : QHttpServerResponder::StatusCode::Ok;

    headers.append("Content-Type", mimeType.name().toUtf8());
    //headers.append("Content-Type", "application/octet-stream");
    //headers.append("Content-Length", QByteArray::number(contentLength));
    headers.append("Content-Disposition", QString("inline; filename=\"%1\"").arg(info.fileName().toUtf8().toPercentEncoding()));
    headers.append("Accept-Ranges", "bytes");

    if (!rangeHeader.isEmpty()) {
        headers.append("Content-Range", QString("bytes %1-%2/%3")
                           .arg(start)
                           .arg(end)
                           .arg(fileSize)
                           .toUtf8());
    }

    responder.write(file, headers, status);
    file->close();
    delete file;
}


#endif // HTTPHELPER_H
