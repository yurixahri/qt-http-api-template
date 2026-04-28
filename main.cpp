#include <QCoreApplication>
#include <QHttpServer>
#include <QTcpServer>
#include <QHttpServerResponse>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "http_components/httpHelper.h"
#include "http_components/htmlTemplate.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QHttpServer server;
    QTcpServer tcpServer;
    uint16_t port = 8080;

    // Admin demo, lock external ip, only work on localhost. You can add your own authenticate for further checking
    server.route("/admin", [](const QHttpServerRequest &request) {
        if (!request.remoteAddress().isLoopback()) {
            return sendStatus("Forbidden", QHttpServerResponse::StatusCode::Forbidden);
        }
        /*
            I was struggling to choose with style of frontend I want to use, I end up using svelte.
            You can custom endpoint yourself to send htmx component, or the old MVC way with inja template

            htmx version:
                QString html = "<ul>";
                for (auto &drive : drives) {
                    html += "<li>" + drive.toHtmlEscaped() + "</li>";
                }
                html += "</ul>";
                QHttpServerResponse response("text/html; charset=utf-8", html, QHttpServerResponse::StatusCode::Ok);
            mvc version:
                QString html = mainPage("Hello"); //see example in htmlTemplate.h
                QHttpServerResponse response("text/html; charset=utf-8", html, QHttpServerResponse::StatusCode::Ok);

            see the httpTemplate.h for reference
        */
        return sendStatus(QHttpServerResponse::StatusCode::Ok);
    });

    // Catch all route
    server.route("/api/*", [](const QHttpServerRequest &request) {
        return sendStatus(QHttpServerResponse::StatusCode::Ok);
    });

    // POST method required OPTION method if the message is JSON
    server.route("/post", QHttpServerRequest::Method::Options,
                 [](const QHttpServerRequest &) {
                     return sendStatus(QHttpServerResponse::StatusCode::Ok);
                 });

    server.route("/post", QHttpServerRequest::Method::Post, [](const QHttpServerRequest &request) {
        if (auto isInvalid = isContentTypeInvalid(request, "application/json")){
            return std::move(isInvalid.value());
        }

        QJsonDocument doc;

        if (auto isInvalid = parseBody(doc, request.body())){
            return std::move(isInvalid.value());
        }

        QJsonObject body = doc.object();
        qDebug() << body;
        // Parsing json in QT is quite hard, you should read the document and see some examples to understand it.
        return sendStatus(QHttpServerResponse::StatusCode::Ok);
    });

    // GET json
    server.route("/get", [](const QHttpServerRequest &request) {
        QJsonObject res;
        res["data1"] = 200;
        res["data2"] = "ok";

        QJsonDocument doc(res);
        QHttpServerResponse response("application/json; charset=utf-8", doc.toJson(QJsonDocument::Compact), QHttpServerResponse::StatusCode::Ok);
        response.setHeaders(createHeaders(.cache = false));
        return response;
    });

    // FTP server
    server.route("/ftp/*", [](const QUrl &url, const QHttpServerRequest &request, QHttpServerResponder &responder) {
        QString relative_path = url.toString();
        // do some path finder, like root + relative_path...
        serveFile(relative_path, request, responder); //serve by chunk, save memory
    });

    // Demo page
    server.route("/page", []( const QHttpServerRequest &request) {
        return mainPage("This api is written in c++");
    });

    // upgrade route to websocket, can be intergrated with your own security check...
    server.addWebSocketUpgradeVerifier(&server, [](const QHttpServerRequest &request){
        if (request.url().path() == "/__/ws") {
            return QHttpServerWebSocketUpgradeResponse::accept();
        }
        return QHttpServerWebSocketUpgradeResponse::passToNext();
    });

    QObject::connect(&server, &QAbstractHttpServer::newWebSocketConnection, [&server]() {
        // Grab the pending connection
        std::unique_ptr<QWebSocket> wsPtr = server.nextPendingWebSocketConnection();
        if (!wsPtr) return;

        // Transfer ownership to Qt's parent/cleanup model
        QWebSocket *ws = wsPtr.release();         // release from unique_ptr
        ws->setParent(&server);                   // or manage in your own container

        // qDebug() << "WebSocket connected";

        QObject::connect(ws, &QWebSocket::textMessageReceived, [ws](const QString &message) {
            //parse json or text
        });

        QObject::connect(ws, &QWebSocket::binaryMessageReceived, [ws](const QByteArray &data) {
            //parse binary
        });

        QObject::connect(ws, &QWebSocket::disconnected, [ws]() {
            //disconnect
        });
    });

    if (!tcpStart(tcpServer, server, port)) return -1;
    logNormal("API: http://localhost:"+QString::number(port).toStdString()+"/");

    return a.exec();
}
