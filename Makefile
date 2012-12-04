CC=gcc
RM=rm -f
CFLAGS=-g -O3 -std=gnu99

SRCS=main.c tester.c scheduler.c calculation.c
OBJS=$(subst .c,.o,$(SRCS))

all: rsj

rsj: $(OBJS)
	gcc $(LDFLAGS) -o rsj $(OBJS)

rjs.o: main.c tester.o scheduler.o debug.h calculation.o structures.o

tester.o: tester.c tester.h

scheduler.o: scheduler.c scheduler.h
scheduler.o: scheduler.c scheduler.c
tester.o: tester.c tester.h
calculation.o: calculation.c calculation.h
structures.o: structures.h

clean:
	$(RM) $(OBJS)

dist-clean: clean
	$(RM) tool

