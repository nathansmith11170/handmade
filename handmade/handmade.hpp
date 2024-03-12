/*
<name TBD>, a handmade game about simulating things in space
Copyright (C) 2024 Nathan Smith

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once
#include <array>
#include <cstdint>
#include <vector>

struct GameOffscreenBuffer {
    std::vector<unsigned char> memory;
    int height, width, pitch;
};

struct GameSoundBuffer {
    std::vector<unsigned char> memory;
    int samplesPerSec;
    int t;
    int bytesPerSample;
    int samplesNeeded;
    bool enabled;
};

struct GameButtonState {
    int halfTransitionCount;
    bool endedDown;
};

struct GameInput {
    GameButtonState speedUp;
    GameButtonState speedDown;
    GameButtonState strafeLeft;
    GameButtonState strafeRight;
};

struct GameState {
    int blueOffset;
    int greenOffset;
    int toneHz;
};

struct GameMemory {
    bool isInitialized;

    std::array<unsigned char, 64l * 1024l * 1024l> permanentStorage;
    std::array<unsigned char, 4l * 1024l * 1024l * 1024l> transientStorage;
};

struct Time64 {
    uint32_t wholeSeconds;
    uint32_t fraction;
};

uint64_t Time64ToU64(Time64 t);
Time64 Time64AddFloat(Time64 t, float addend);

void renderWeirdGradient(GameState *state, GameOffscreenBuffer *buffer);
void outputSine(GameState *state, GameSoundBuffer *buffer);
void updateGame(GameMemory *memory, GameInput *input);
void fillBuffers(GameMemory *memory, GameOffscreenBuffer *offscreenBuffer,
                 GameSoundBuffer *soundBuffer);
