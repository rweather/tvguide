TARGET = tst_tvbookmark
CONFIG += qtestlib
SOURCES += tst_tvbookmark.cpp
include(../../src/library.pri)

gcov {
    QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
    LIBS += -fprofile-arcs -ftest-coverage
}
