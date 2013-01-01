TEMPLATE = app
CONFIG += console

SOURCES += \
    main.cc \
    rb.c \
    iupdateprocessor.cc \
    init.cc \
    worker.cc \
    globals.cc

HEADERS += \
    debug.h \
    structures.h \
    rb.h \
    iupdateprocessor.h \
    iresultconsumer.h \
    init.h \
    table.h \
    worker.h \
    sync.h \
    helpers.h \
    globals.h

