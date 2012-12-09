TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += \
    scheduler.c \
    main.c \
    tester.c \
    calculation.c \
    worker.c

HEADERS += \
    tester.h \
    scheduler.h \
    debug.h \
    structures.h \
    calculation.h \
    error.h

