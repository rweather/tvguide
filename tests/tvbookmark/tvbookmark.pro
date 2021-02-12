QT += testlib
TARGET = tst_tvbookmark
CONFIG += testlib
SOURCES += tst_tvbookmark.cpp
include(../../src/library.pri)

gcov {
    QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
    LIBS += -fprofile-arcs -ftest-coverage
}
