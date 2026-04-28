#ifndef STRING_UTILS_H
#define STRING_UTILS_H


#include <vector>
#include <QString>
#include <QStringList>
#include <QRandomGenerator>
#include <iostream>
#include <string>
#include <sstream>
#include "utils/rang.hpp"

using namespace std;
using namespace rang;

QString joinVecToString(const std::vector<QString>& vec, const QString delimiter, bool mode = false);
QString generateRandomString(int length);
bool QStringIncludes(const QString& string, const QStringList& list);
int readOptionalInt(const std::string &prompt, int defaultValue);
void logSuccess(const std::string msg);
void logWarning(const std::string msg);
void logError(const std::string msg);
void logNormal(const std::string msg, bool endLine = true);
#endif
