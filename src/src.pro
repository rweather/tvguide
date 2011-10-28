TARGET = tvguide

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    bookmarkitemeditor.cpp \
    bookmarklisteditor.cpp \
    categoryselector.cpp \
    channeleditor.cpp \
    dayselectiondialog.cpp \
    helpbrowser.cpp \
    programmeview.cpp \
    serviceeditor.cpp \
    serviceselector.cpp \
    websearchdialog.cpp \

HEADERS += \
    mainwindow.h \
    bookmarkitemeditor.h \
    bookmarklisteditor.h \
    categoryselector.h \
    channeleditor.h \
    dayselectiondialog.h \
    helpbrowser.h \
    programmeview.h \
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
    categoryselector.ui \
    channeleditor.ui \
    dayselectiondialog.ui \
    helpbrowser.ui \
    serviceeditor.ui \
    serviceselector.ui \
    websearchdialog.ui \
