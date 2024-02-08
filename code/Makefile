CFlags = -Wall -Wpedantic -pedantic-errors -Wall -Wextra -Wconversion -Wsign-conversion -std=c2x -I/usr/include/SDL2 -D_REENTRANT

LINKERFLAG = -L/usr/include/SDL2 -lSDL2 -lSDL2_ttf -lm

CC = clang

DEBUG = -g
RELEASE = -O2 -DNDEBUG

handmade:
	$(CC) $(CFlags) $(LINKERFLAG) $(DEBUG) sdl_handmade.c -o ../build/handmade

clean:
	rm ../build/handmade
