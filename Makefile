CC=gcc
TARGETS = screen screen_cpu
CFLAGS = -g -std=c11 -Wfatal-errors -DGL_SILENCE_DEPRECATION
LFLAGS = -g -L/usr/local/lib -lm
INCLUDES=
HEADERS:=$(wildcard *.h) Makefile

OS:=$(shell uname)
ifeq ($(OS),Darwin) #OSX
  GL_FLAGS=-lglew -lglfw -framework Cocoa -framework OpenGL -lpthread
  CFLAGS:=$(CFLAGS) -DAPPLE
else # Linux or other
  GL_FLAGS=-lglfw -lGL -lpthread
endif

.SUFFIXES:

all: $(TARGETS)

%.o: %.c $(HEADERS)
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

%: %.o mem.o ops.o alu.o ldrr.o
	$(CC) -o $@ $^ $(LFLAGS) $(GL_FLAGS)

clean:
	rm -rf $(TARGETS) *.o
