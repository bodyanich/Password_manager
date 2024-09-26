TARGET = ToughPass

QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Dropbox/dropboxapi.cpp \
    Dropbox/dropboxfolderlist.cpp \
    Dropbox/uploaddbdialog.cpp \
    Random.cpp \
    authcodedialog.cpp \
    createdbwidget.cpp \
    database.cpp \
    databasewidget.cpp \
    editentry.cpp \
    editgroup.cpp \
    main.cpp \
    mainwindow.cpp \
    passworddialog.cpp \
    passwordgeneratorcore.cpp \
    passwordgenereatorwidget.cpp \
    unlockwidget.cpp \
    welcomewidget.cpp \
    winDark.cpp \
    zxcvbn/zxcvbn.cpp

HEADERS += \
    Dropbox/dropboxapi.h \
    Dropbox/dropboxfolderlist.h \
    Dropbox/uploaddbdialog.h \
    Random.h \
    authcodedialog.h \
    createdbwidget.h \
    database.h \
    databasewidget.h \
    editentry.h \
    editgroup.h \
    mainwindow.h \
    passworddialog.h \
    passwordgeneratorcore.h \
    passwordgenereatorwidget.h \
    unlockwidget.h \
    welcomewidget.h \
    winDark.h \
    zxcvbn/dict-src.h \
    zxcvbn/zxcvbn.h

FORMS += \
    Dropbox/dropboxfolderlist.ui \
    Dropbox/uploaddbdialog.ui \
    authcodedialog.ui \
    createdbwidget.ui \
    databasewidget.ui \
    editentry.ui \
    editgroup.ui \
    mainwindow.ui \
    passworddialog.ui \
    passwordgenereatorwidget.ui \
    unlockwidget.ui \
    welcomewidget.ui

TRANSLATIONS += \
    LightPass_en_US.ts
CONFIG += lrelease
CONFIG += embed_translations

DEFINES += SODIUM_STATIC
INCLUDEPATH += C:\libsodium\include
LIBS += -LC:\libsodium\lib -llibsodium

INCLUDEPATH += C:\zxcvbn-c

RC_ICONS = Icons/logo.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

