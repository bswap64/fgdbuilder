QT += core gui widgets

CONFIG += c++17

TARGET = fgdbuilder
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/MainWindow.cpp \
    src/EntityEditor.cpp \
    src/KeyvalueEditor.cpp \
    src/IOEditor.cpp \
    src/FlagEditor.cpp \
    src/FGDGenerator.cpp \
    src/BaseClassManager.cpp \
    src/IncludeManager.cpp \
    src/AutoVisGroupEditor.cpp \
    src/MapSizeDialog.cpp

HEADERS += \
    src/MainWindow.h \
    src/EntityEditor.h \
    src/KeyvalueEditor.h \
    src/IOEditor.h \
    src/FlagEditor.h \
    src/FGDGenerator.h \
    src/BaseClassManager.h \
    src/IncludeManager.h \
    src/AutoVisGroupEditor.h \
    src/MapSizeDialog.h \
    src/FGDData.h

RC_FILE = fgdbuilder.rc

RESOURCES += resources.qrc
