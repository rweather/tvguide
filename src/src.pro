TARGET = tvguide
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    bookmarkitemeditor.cpp \
    bookmarklisteditor.cpp \
    channeleditor.cpp \
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

FORMS += \
    mainwindow.ui \
    bookmarkitemeditor.ui \
    bookmarklisteditor.ui \
    channeleditor.ui \
    serviceeditor.ui \
    serviceselector.ui \

QT += network
