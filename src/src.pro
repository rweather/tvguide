TARGET = tvguide
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    channeleditor.cpp \
    tvchannel.cpp \
    tvchannellist.cpp \
    tvchannelmodel.cpp \
    tvprogramme.cpp \
    tvprogrammedelegate.cpp \
    tvprogrammemodel.cpp \

HEADERS += \
    mainwindow.h \
    channeleditor.h \
    tvchannel.h \
    tvchannellist.h \
    tvchannelmodel.h \
    tvprogramme.h \
    tvprogrammedelegate.h \
    tvprogrammemodel.h \

RESOURCES += \
    tvguide.qrc \

FORMS += \
    mainwindow.ui \
    channeleditor.ui \

QT += network
