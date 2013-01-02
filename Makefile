
# Makefile for building: rsj
# Generated by qmake (2.01a) (Qt 4.8.2) on: Tue Jan 1 19:26:57 2013
# Project:  rsj.pro
# Template: app
# Command: /usr/bin/qmake-qt4 -spec /usr/share/qt4/mkspecs/linux-g++ -o Makefile rsj.pro
#############################################################################

####### Compiler, tools and options

CC            = gcc
CXX           = g++
LIBS          = $(SUBLIBS)  -L/usr/lib/x86_64-linux-gnu -I include -lpthread -lrt -I include/gperftools -L include -ltcmalloc_minimal
DEFINES       =
CFLAGS        = -pipe -O2 -march=corei7 $(DEFINES) $(LIBS) -DNDEBUG
CXXFLAGS      = -pipe -O2 -march=corei7 $(DEFINES) $(LIBS) -DNDEBUG -std=c++0x
INCPATH       = 
LINK          = g++ $(LIBS) -flto
LFLAGS        = -Wl,-O2
AR            = ar cqs
RANLIB        = 
QMAKE         = 
TAR           = tar -cf
COMPRESS      = gzip -9f
COPY          = cp -f
SED           = sed
COPY_FILE     = $(COPY)
COPY_DIR      = $(COPY) -r
STRIP         = strip
INSTALL_FILE  = install -m 644 -p
INSTALL_DIR   = $(COPY_DIR)
INSTALL_PROGRAM = install -m 755 -p
DEL_FILE      = rm -f
SYMLINK       = ln -f -s
DEL_DIR       = rmdir
MOVE          = mv -f
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p

####### Output directory

OBJECTS_DIR   = ./

####### Files

SOURCES       = main.cc \
		rb.c \
		iupdateprocessor.cc \
		init.cc \
		worker.cc \
		globals.cc 
OBJECTS       = main.o \
		rb.o \
		iupdateprocessor.o \
		init.o \
		worker.o \
		globals.o
QMAKE_TARGET  = rsj
DESTDIR       = 
TARGET        = rsj

first: all
####### Implicit rules

.SUFFIXES: .o .c .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"

####### Build rules

all: Makefile $(TARGET)

$(TARGET):  $(OBJECTS)  
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LIBS)
dist: 
	@$(CHK_DIR_EXISTS) .tmp/rsj1.0.0 || $(MKDIR) .tmp/rsj1.0.0 
	$(COPY_FILE) --parents $(SOURCES) $(DIST) .tmp/rsj1.0.0/ && $(COPY_FILE) --parents debug.h structures.h rb.h iupdateprocessor.h iresultconsumer.h init.h table.h worker.h sync.h helpers.h globals.h .tmp/rsj1.0.0/ && $(COPY_FILE) --parents main.cc rb.c iupdateprocessor.cc init.cc worker.cc globals.cc .tmp/rsj1.0.0/ && (cd `dirname .tmp/rsj1.0.0` && $(TAR) rsj1.0.0.tar rsj1.0.0 && $(COMPRESS) rsj1.0.0.tar) && $(MOVE) `dirname .tmp/rsj1.0.0`/rsj1.0.0.tar.gz . && $(DEL_FILE) -r .tmp/rsj1.0.0


####### Sub-libraries

distclean: clean
	-$(DEL_FILE) $(TARGET) 
	-$(DEL_FILE) Makefile


check: first

mocclean: compiler_moc_header_clean compiler_moc_source_clean

mocables: compiler_moc_header_make_all compiler_moc_source_make_all

####### Compile

main.o: main.cc worker.h \
		structures.h \
		rb.h \
		iresultconsumer.h \
		globals.h \
		debug.h \
		iupdateprocessor.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o main.o main.cc

rb.o: rb.c rb.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o rb.o rb.c

iupdateprocessor.o: iupdateprocessor.cc iupdateprocessor.h \
		iresultconsumer.h \
		structures.h \
		rb.h \
		worker.h \
		globals.h \
		debug.h \
		init.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o iupdateprocessor.o iupdateprocessor.cc

init.o: init.cc worker.h \
		structures.h \
		rb.h \
		iresultconsumer.h \
		globals.h \
		debug.h \
		table.h \
		helpers.h \
		sync.h \
		init.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o init.o init.cc

worker.o: worker.cc rb.h \
		debug.h \
		structures.h \
		iresultconsumer.h \
		helpers.h \
		table.h \
		sync.h \
		globals.h \
		worker.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o worker.o worker.cc

globals.o: globals.cc globals.h \
		structures.h \
		rb.h \
		iresultconsumer.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o globals.o globals.cc

####### Install

install:   FORCE

uninstall:   FORCE

FORCE:

