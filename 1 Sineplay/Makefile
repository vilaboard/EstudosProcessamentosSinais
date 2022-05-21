CC=gcc
CFLAGS=-g -O2 -Wall $(shell sdl2-config --cflags)
LDFLAGS=$(shell sdl2-config --libs) -lSDL2_mixer -lm

PROGRAMS=$(basename $(wildcard *.c))

%: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@ $(LDFLAGS)

all: $(PROGRAMS)

clean:
	rm -f $(PROGRAMS) *.o

new: clean all
