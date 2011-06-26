TARGET = tvguide

TVGUIDE_VERSION = 0.0.2

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    bookmarkitemeditor.cpp \
    bookmarklisteditor.cpp \
    channeleditor.cpp \
    helpbrowser.cpp \
    serviceeditor.cpp \
    serviceselector.cpp \
    tvbookmark.cpp \
    tvbookmarkmodel.cpp \
    tvchannel.cpp \
    tvchannellist.cpp \
    tvchannelmodel.cpp \
    tvprogramme.cpp \
    tvprogrammedelegate.cpp \
    tvprogrammemodel.cpp \

HEADERS += \
    mainwindow.h \
    bookmarkitemeditor.h \
    bookmarklisteditor.h \
    channeleditor.h \
    helpbrowser.h \
    serviceeditor.h \
    serviceselector.h \
    tvbookmark.h \
    tvbookmarkmodel.h \
    tvchannel.h \
    tvchannellist.h \
    tvchannelmodel.h \
    tvprogramme.h \
    tvprogrammedelegate.h \
    tvprogrammemodel.h \

RESOURCES += \
    tvguide.qrc \
    $$PWD/../help/en/help_en.qrc \

FORMS += \
    aboutdialog.ui \
    mainwindow.ui \
    bookmarkitemeditor.ui \
    bookmarklisteditor.ui \
    channeleditor.ui \
    helpbrowser.ui \
    serviceeditor.ui \
    serviceselector.ui \

QT += network
DEFINES += TVGUIDE_VERSION=\\\"$$TVGUIDE_VERSION\\\"
