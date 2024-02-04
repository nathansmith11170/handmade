#!/bin/bash

c++ ./code/sdl_handmade.cpp -o ./build/handmade -ggdb -std=c++17 -Wpedantic -pedantic-errors -Wall -Wextra -Wconversion -Wsign-conversion -I/usr/include/SDL2 -D_REENTRANT -lSDL2 -lSDL2_ttf
