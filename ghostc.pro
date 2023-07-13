#-------------------------------------------------
#
# Project created by QtCreator 2014-01-05T02:17:40
#
#-------------------------------------------------

QT       += core network script sql
QT       -= gui

QMAKE_INSTALL_FILE = install -m 640 -p -g root
QMAKE_INSTALL_PROGRAM = install -m 750 -p -g ghost

TARGET = ghostc
CONFIG   += console
CONFIG   -= app_bundle

LIBS += -L/usr/lib/x86_64-linux-gnu -lqca
INCLUDEPATH += /usr/include/QtCrypto

TEMPLATE = app

SOURCES += main.cpp \
    gmailer.cpp \
    dbhelper.cpp \
    smtpclient/emailaddress.cpp \
    smtpclient/mimeattachment.cpp \
    smtpclient/mimecontentformatter.cpp \
    smtpclient/mimefile.cpp \
    smtpclient/mimehtml.cpp \
    smtpclient/mimeinlinefile.cpp \
    smtpclient/mimemessage.cpp \
    smtpclient/mimemultipart.cpp \
    smtpclient/mimepart.cpp \
    smtpclient/mimetext.cpp \
    smtpclient/quotedprintable.cpp \
    smtpclient/smtpclient.cpp \
    smtphelper.cpp \
    cryptohelper.cpp \
    ipapihelper.cpp \
    gadmin.cpp \
    ghostc.cpp \
    validator.cpp \
    iohelper.cpp \

HEADERS += \
    gmailer.h \
    dbhelper.h \
    smtpclient/SmtpMime \
    smtpclient/emailaddress.h \
    smtpclient/mimeattachment.h \
    smtpclient/mimecontentformatter.h \
    smtpclient/mimefile.h \
    smtpclient/mimehtml.h \
    smtpclient/mimeinlinefile.h \
    smtpclient/mimemessage.h \
    smtpclient/mimemultipart.h \
    smtpclient/mimepart.h \
    smtpclient/mimetext.h \
    smtpclient/quotedprintable.h \
    smtpclient/smtpclient.h \
    smtphelper.h \
    cryptohelper.h \
    ipapihelper.h \
    gadmin.h \
    ghostc.h \
    validator.h \
    iohelper.h

OTHER_FILES += \
    ghost.cfg \
    README.txt

cfg.path = /etc/ghost/
cfg.files = ghost.cfg

exe.path = /usr/bin/
exe.files = ghostc

INSTALLS += cfg exe
