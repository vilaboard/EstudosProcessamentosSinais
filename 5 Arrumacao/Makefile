CC=gcc
CFLAGS=-g -Wall $(shell sdl2-config --cflags) -march=native
LDFLAGS=$(shell sdl2-config --libs) -lSDL2_mixer -lm -lfftw3f

PROGRAMS=$(basename $(wildcard *.c))

%: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@ $(LDFLAGS)

all: $(PROGRAMS)

check:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --leak-check=yes ./$(PROGRAMS)

clean:
	rm -f $(PROGRAMS) *.o

new: clean all
