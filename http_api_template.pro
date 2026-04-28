QT = core network httpserver

CONFIG += c++17 cmdline static

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCE_VIEWS_DIR = $$quote($$PWD/views)
CONFIG(debug, debug|release) {
    # Path for Debug mode
    DEST_VIEWS_DIR = $$OUT_PWD/debug/views
} else {
    # Path for Release mode
    DEST_VIEWS_DIR = $$OUT_PWD/release/views
}

win32 {
    SOURCE_VIEWS_DIR ~= s,/,\\,g
    DEST_VIEWS_DIR   ~= s,/,\\,g
}

win32 {
    copy_views.commands = xcopy /s /q /y /i $$quote($$SOURCE_VIEWS_DIR) $$quote($$DEST_VIEWS_DIR)
} else {
    copy_views.commands = $(MKDIR) $$quote($$DEST_VIEWS_DIR) && $(COPY_DIR) $$quote($$SOURCE_VIEWS_DIR) $$quote($$DEST_VIEWS_DIR)
}

copy_views.depends = $(first)
QMAKE_EXTRA_TARGETS += copy_views
PRE_TARGETDEPS += copy_views

SOURCES += \
        http_components/webSocketHelper.cpp \
        main.cpp \
        utils/stringUtils.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    http_components/htmlTemplate.h \
    http_components/httpHelper.h \
    http_components/webSocketHelper.h \
    utils/fs.h \
    utils/inja.hpp \
    utils/json.hpp \
    utils/log.h \
    utils/rang.hpp \
    utils/stringUtils.h

RESOURCES += \
    resources.qrc
