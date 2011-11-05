TVGUIDE_VERSION = 0.1.3

VPATH += $$PWD
INCLUDEPATH += $$PWD

SOURCES += \
    tvbookmark.cpp \
    tvbookmarklist.cpp \
    tvbookmarkmodel.cpp \
    tvchannel.cpp \
    tvchanneleditmodel.cpp \
    tvchannellist.cpp \
    tvchannelmodel.cpp \
    tvprogramme.cpp \
    tvprogrammedelegate.cpp \
    tvprogrammefilter.cpp \
    tvprogrammemodel.cpp \
    tvtick.cpp \

HEADERS += \
    tvbookmark.h \
    tvbookmarklist.h \
    tvbookmarkmodel.h \
    tvchannel.h \
    tvchanneleditmodel.h \
    tvchannellist.h \
    tvchannelmodel.h \
    tvprogramme.h \
    tvprogrammedelegate.h \
    tvprogrammefilter.h \
    tvprogrammemodel.h \
    tvtick.h \

QT += network
DEFINES += TVGUIDE_VERSION=\\\"$$TVGUIDE_VERSION\\\"
