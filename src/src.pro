TARGET = tvguide

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    bookmarkitemeditor.cpp \
    bookmarklisteditor.cpp \
    channeleditor.cpp \
    helpbrowser.cpp \
    serviceeditor.cpp \
    serviceselector.cpp \
    websearchdialog.cpp \

HEADERS += \
    mainwindow.h \
    bookmarkitemeditor.h \
    bookmarklisteditor.h \
    channeleditor.h \
    helpbrowser.h \
    serviceeditor.h \
    serviceselector.h \
    websearchdialog.h \

include(library.pri)

RESOURCES += \
    tvguide.qrc \
    $$PWD/../help/en/help_en.qrc \
    $$PWD/../data/data.qrc \

FORMS += \
    aboutdialog.ui \
    mainwindow.ui \
    bookmarkitemeditor.ui \
    bookmarklisteditor.ui \
    channeleditor.ui \
    helpbrowser.ui \
    serviceeditor.ui \
    serviceselector.ui \
    websearchdialog.ui \
