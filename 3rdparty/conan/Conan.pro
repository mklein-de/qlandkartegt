# -------------------------------------------------
# Project created by QtCreator 2009-06-18T19:49:43
# -------------------------------------------------
TARGET = Conan
TEMPLATE = lib
CONFIG(debug, debug|release):TARGET = $$join(TARGET,,,d)
CONFIG += dll
VERSION = 1.0.2
DEFINES += CONAN_DLL \
    CONAN_DLL_EXPORTS
linux-g++: QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
DESTDIR = lib
MOC_DIR = src
RCC_DIR = src
UI_DIR = src
OBJECTS_DIR = obj
SOURCES += src/ObjectUtility.cpp \
    src/SignalSpy.cpp \
    src/ConnectionModel.cpp \
    src/ObjectModel.cpp \
    src/ConanWidget.cpp \
    src/AboutDialog.cpp \
    src/ConanDebug.cpp \
    src/KeyValueTableModel.cpp
HEADERS += src/ConanDefines.h \
    src/ObjectUtility.h \
    src/SignalSpy.h \
    src/ConnectionModel.h \
    src/ConanWidget_p.h \
    src/ConanWidget.h \
    src/ConanDefines.h \
    src/AboutDialog.h \
    src/ObjectModel.h \
    src/ConanDebug.h \
    src/KeyValueTableModel.h \
    include/Conan.h
FORMS += src/ConanWidget.ui \
    src/AboutDialog.ui
RESOURCES += src/Conan.qrc
OTHER_FILES += GPL.txt \
    CHANGES.txt \
    TODO.txt \
    README.txt \
    icons/zoom.png \
    icons/transmit_blue.png \
    icons/resultset_previous.png \
    icons/resultset_next.png \
    icons/README.txt \
    icons/page_save.png \
    icons/lock.png \
    icons/key.png \
    icons/information.png \
    icons/find.png \
    icons/delete.png \
    icons/connect.png \
    icons/chart_organisation.png \
    icons/CCL.txt \
    icons/camera.png \
    icons/bug.png \
    icons/arrow_refresh.png
