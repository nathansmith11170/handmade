#!/bin/bash

gcc ./code/sdl_handmade.c -o ./build/handmade -ggdb -std=c2x -Wpedantic -pedantic-errors -Wall -Wextra -Wconversion -Wsign-conversion -I/usr/include/SDL2 -D_REENTRANT -lSDL2 -lSDL2_ttf
