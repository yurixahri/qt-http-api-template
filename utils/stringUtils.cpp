#include "stringUtils.h"

QString joinVecToString(const std::vector<QString>& vec, const QString delimiter, bool mode){
    QString _Temp;
    for (const QString& s : vec){
        _Temp += s+delimiter;
    }
    if (mode) _Temp.chop(1);
    return _Temp;

};


bool QStringListIncludes(const QString& string, const QStringList& list){
    for (const QString& s : list){
        if (string == s) {
            return 1;
            break;
        }
    }
    return 0;
}

int readOptionalInt(const std::string &prompt, int defaultValue) {
    std::string line;
    logNormal(prompt, false);
    std::getline(std::cin, line);

    if (line.empty()) {
        return defaultValue;
    }

    std::istringstream iss(line);
    int value;
    if (iss >> value) {
        return value;
    }

    std::cerr << "Invalid input. Using default value.\n";
    return defaultValue;
}

void logSuccess(const std::string msg){
    cout <<  style::bold << bg::green << fg::gray << msg << fg::reset << bg::reset << style::reset << endl;
}

void logWarning(const std::string msg){
    cout <<  style::bold << bg::yellow << fg::gray << msg << fg::reset << bg::reset << style::reset << endl;
}

void logError(const std::string msg){
    cout <<  style::bold << bg::red << fg::gray << msg << fg::reset << bg::reset << style::reset << endl;
}

void logNormal(const std::string msg, bool endLine){
    cout << msg ;
    if (endLine) cout << endl;
}
