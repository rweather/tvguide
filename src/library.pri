TVGUIDE_VERSION = 0.0.7

VPATH += $$PWD
INCLUDEPATH += $$PWD

SOURCES += \
    tvbookmark.cpp \
    tvbookmarkmodel.cpp \
    tvchannel.cpp \
    tvchannellist.cpp \
    tvchannelmodel.cpp \
    tvprogramme.cpp \
    tvprogrammedelegate.cpp \
    tvprogrammemodel.cpp \

HEADERS += \
    tvbookmark.h \
    tvbookmarkmodel.h \
    tvchannel.h \
    tvchannellist.h \
    tvchannelmodel.h \
    tvprogramme.h \
    tvprogrammedelegate.h \
    tvprogrammemodel.h \

QT += network
DEFINES += TVGUIDE_VERSION=\\\"$$TVGUIDE_VERSION\\\"
