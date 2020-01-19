.SUFFIXES:
TARGETS=test80

AS=gcc
CC=gcc
LD=gcc
CFLAGS=-g -Wfatal-errors -O1
LFLAGS=-g
AFLAGS=
HEADERS:=$(wildcard *.h) Makefile

all: $(TARGETS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@
%.o: %.s
	$(AS) $(AFLAGS) -c $< -o $@

test80: test80.o alu.o ldrr.o util.o ops.o emu.o
	$(CC) $(CLAGS) $^ -o $@

clean:
	rm -rf *.o *.dSYM $(TARGETS)
