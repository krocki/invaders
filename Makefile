.SUFFIXES:
TARGETS=test80

CC=gcc
LD=gcc
CFLAGS=-g -Wfatal-errors -O0
LFLAGS=-g
HEADERS:=$(wildcard *.h) Makefile

all: $(TARGETS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

test80: test80.o alu.o ldrr.o util.o ops.o
	$(CC) $(CLAGS) $^ -o $@

clean:
	rm -rf *.o *.dSYM $(TARGETS)
