#-------------------------------------------------
# ChatClient - WeChat-like IM Client
# Qt6 qmake project file for macOS
#-------------------------------------------------

QT       += core gui widgets network websockets multimedia protobuf

CONFIG   += c++17
CONFIG   -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs
# to ensure your code is portable across all Qt versions.
# DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
    main.cpp \
    mainwidget.cpp \
    sessionfriendarea.cpp \
    messageshowarea.cpp \
    messageeditarea.cpp \
    selfinfowidget.cpp \
    userinfowidget.cpp \
    sessiondetailwidget.cpp \
    choosefrienddialog.cpp \
    groupsessiondetailwidget.cpp \
    addfrienddialog.cpp \
    historymessagewidget.cpp \
    loginwidget.cpp \
    phoneloginwidget.cpp \
    toast.cpp \
    model/datacenter.cpp \
    network/NetClient.cpp \
    verifycodewidget.cpp \
    soundrecorder.cpp \
    proto/base.qpb.cpp \
    proto/file.qpb.cpp \
    proto/friend.qpb.cpp \
    proto/gateway.qpb.cpp \
    proto/message_storage.qpb.cpp \
    proto/message_transmit.qpb.cpp \
    proto/notify.qpb.cpp \
    proto/speech_recognition.qpb.cpp \
    proto/user.qpb.cpp

HEADERS += \
    mainwidget.h \
    sessionfriendarea.h \
    messageshowarea.h \
    messageeditarea.h \
    selfinfowidget.h \
    userinfowidget.h \
    sessiondetailwidget.h \
    choosefrienddialog.h \
    groupsessiondetailwidget.h \
    addfrienddialog.h \
    historymessagewidget.h \
    loginwidget.h \
    phoneloginwidget.h \
    toast.h \
    debug.h \
    model/data.h \
    model/datacenter.h \
    network/NetClient.h \
    verifycodewidget.h \
    soundrecorder.h \
    proto/base.qpb.h \
    proto/file.qpb.h \
    proto/friend.qpb.h \
    proto/gateway.qpb.h \
    proto/message_storage.qpb.h \
    proto/message_transmit.qpb.h \
    proto/notify.qpb.h \
    proto/speech_recognition.qpb.h \
    proto/user.qpb.h

FORMS += \
    mainwidget.ui

RESOURCES += \
    resource.qrc

# =================================================
# Protobuf Configuration
# =================================================
# Proto files are pre-compiled with qtprotobufgen.
# To regenerate, run: ./generate_proto.sh

# =================================================
# Platform-specific settings
# =================================================

macx {
    # macOS specific settings
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 12.0

    # For linking with frameworks installed via Homebrew
    # Uncomment and adjust if needed:
    # INCLUDEPATH += /opt/homebrew/include
    # LIBS += -L/opt/homebrew/lib
}

win32 {
    # Windows specific settings
    CONFIG += windows
}

# =================================================
# Include paths
# =================================================

INCLUDEPATH += . \
    ./model \
    ./network \
    ./proto

# =================================================
# Build output directories
# =================================================

# By default, Qt Creator uses shadow builds (build directory separate from source)
# The following are defaults that can be overridden:
# OBJECTS_DIR = build/obj
# MOC_DIR = build/moc
# RCC_DIR = build/rcc
# UI_DIR = build/ui
# DESTDIR = build/bin
