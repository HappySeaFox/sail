!equals(QT_MAJOR_VERSION, 5) {
    error("This project requires Qt version 5 only.")
}

TEMPLATE = app
TARGET = qt-sail-demo-hacker

CONFIG += warn_on c++11 strict_c++ link_pkgconfig
CONFIG -= embed_manifest_exe
QT += widgets

PKGCONFIG += libsail libsail-common

HEADERS += qtsail.h \
    writeoptions.h
SOURCES += main.cpp qtsail.cpp \
    writeoptions.cpp
FORMS   += qtsail.ui \
    writeoptions.ui

win32 {
    RC_FILE = qtsail.rc
}

# Default SAIL installation on Windows
#
win32 {
    LIBS += -LC:/SAIL/bin
}

# Enable address sanitizer (requires GCC >= 4.8)
#
*-g++ {
    OPT=-fsanitize=address,leak,undefined,shift,shift-exponent,shift-base,integer-divide-by-zero,unreachable,vla-bound,null,signed-integer-overflow,bounds-strict,alignment,object-size,float-divide-by-zero,float-cast-overflow,nonnull-attribute,returns-nonnull-attribute,bool,enum,vptr
    QMAKE_CXX_FLAGS += $$OPT
    QMAKE_LFLAGS += $$OPT

    QMAKE_CXXFLAGS += -Wall -Wextra
    QMAKE_CFLAGS += -Wall -Wextra
}
