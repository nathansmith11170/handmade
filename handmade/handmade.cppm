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
export module Handmade;

import std;

export struct DebugReadFileResult {
    void *data;
    int fileSize;
};

export struct GameOffscreenBuffer {
    std::vector<std::uint32_t> memory;
    std::int32_t height, width, pitch;
};

export struct GameSoundBuffer {
    std::vector<std::int16_t> memory;
    std::int32_t samplesPerSec;
    std::int32_t t;
    std::int32_t bytesPerSample;
    std::int32_t samplesNeeded;
    bool enabled;
};

export struct GameButtonState {
    std::int32_t halfTransitionCount;
    bool endedDown;
};

export struct GameInput {
    GameButtonState speedUp;
    GameButtonState speedDown;
    GameButtonState strafeLeft;
    GameButtonState strafeRight;
};

export struct GameState {
    std::int32_t blueOffset;
    std::int32_t greenOffset;
    std::int32_t toneHz;
};

export struct GameMemory {
    bool isInitialized;

    std::array<std::uint8_t, 64l * 1024l * 1024l> permanentStorage;
    std::array<std::uint8_t, 4l * 1024l * 1024l * 1024l> transientStorage;
};

export struct Time64 {
    std::uint32_t wholeSeconds;
    std::uint32_t fraction;
};

export std::uint64_t Time64ToU64(Time64 t);
export Time64 Time64AddFloat(Time64 t, float addend);

export void renderWeirdGradient(GameState *state, GameOffscreenBuffer *buffer);
export void outputSine(GameState *state, GameSoundBuffer *buffer);
export void updateGame(GameMemory *memory, GameInput *input);
export void fillBuffers(GameMemory *memory,
                        GameOffscreenBuffer *offscreenBuffer,
                        GameSoundBuffer *soundBuffer);
