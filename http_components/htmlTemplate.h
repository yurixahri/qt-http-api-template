#ifndef HTMLTEMPLATE_H
#define HTMLTEMPLATE_H

#include "utils/inja.hpp"
#include "utils/stringUtils.h"
#include <QString>
#include <QStringList>
#include <QUrl>
using namespace inja;
Environment env;

/*
    MVC style using inja
    https://github.com/pantor/inja
*/

QString mainPage(QString msg){
    json data;
    data["msg"] = msg.toStdString();
    return QString::fromStdString(env.render(env.parse_template("./views/index.html"), data));
}

// QString filesPage(QStringList &folders, QStringList &files, QString dir){
//     json data;
//     json fl_arr = json::array();
//     json fi_arr = json::array();


//     for (auto &folder : folders){
//         fl_arr.push_back({
//             {"n", QUrl::toPercentEncoding(folder.toUtf8()).toStdString()},
//             {"n_decoded", folder.toStdString()}
//         });
//     }

//     for (auto &file : files){
//         fi_arr.push_back({
//             {"n", QUrl::toPercentEncoding(file.toUtf8()).toStdString()},
//             {"n_decoded", file.toStdString()}
//         });
//     }

//     data["folders"] = fl_arr;
//     data["files"] = fi_arr;
//     data["dir"] = QUrl::toPercentEncoding(dir.toUtf8()).toStdString();
//     dir.chop(1);
//     QStringList back = dir.split("/");
//     back.removeLast();
//     data["back"] = QUrl::toPercentEncoding(back.join("/").toUtf8()).toStdString();
//     return QString::fromStdString(env.render(env.parse_template("./views/files.html"), data));
// }
#endif // HTMLTEMPLATE_H
