#-------------------------------------------------
#
# Project created by QtCreator 2016-05-28T11:03:26
#
#-------------------------------------------------

QT       += core gui network sql concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mailClient
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    pop3_connection.cpp \
    settingsdialog.cpp \
    accountsmanager.cpp \
    mail_handler.cpp \
    smtp_connection.cpp \
    sendmessage.cpp

HEADERS  += mainwindow.h \
    pop3_connection.h \
    settingsdialog.h \
    accountsmanager.h \
    pop3_commands.h \
    mail_handler.h \
    pop3connectionstate.h \
    smtp_connection.h \
    sendmessage.h

FORMS    += mainwindow.ui \
    sendmessagedialog.ui \
    SettingsDialog.ui \
    accountsManager.ui
