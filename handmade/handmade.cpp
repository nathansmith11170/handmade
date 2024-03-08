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

export namespace Handmade {
struct DebugReadFileResult {
    void *data;
    int fileSize;
};

// static DebugReadFileResult debugPlatformReadEntireFile(const char *fileName);
// static void debugPlatformFreeFileMemory(void *memory);
// static bool debugPlatformWriteEntireFile(const char *fileName, int fileSize,
//                                          void *memory);

struct GameOffscreenBuffer {
    std::uint8_t *memory;
    std::int32_t height, width, pitch;
};

struct GameSoundBuffer {
    std::int16_t *memory;
    std::int32_t samplesPerSec;
    std::int32_t t;
    std::int32_t bytesPerSample;
    std::int32_t samplesNeeded;
    bool enabled;
};

struct GameButtonState {
    std::int32_t halfTransitionCount;
    bool endedDown;
};

struct GameInput {
    GameButtonState speedUp;
    GameButtonState speedDown;
    GameButtonState strafeLeft;
    GameButtonState strafeRight;
};

struct GameState {
    std::int32_t blueOffset;
    std::int32_t greenOffset;
    std::int32_t toneHz;
};

struct GameMemory {
    bool isInitialized;

    std::uint64_t permanentStorageSize;
    std::uint8_t *permanentStorage;

    std::uint64_t transientStorageSize;
    std::uint8_t *transientStorage;
};

void renderWeirdGradient(GameState *state, GameOffscreenBuffer *buffer) {
    std::uint8_t *row{buffer->memory};
    for (int y = 0; y < buffer->height; ++y) {
        std::uint32_t *pixel{(std::uint32_t *)row};
        for (int x = 0; x < buffer->width; ++x) {
            std::uint8_t blue{(std::uint8_t)(x + state->blueOffset)};
            std::uint8_t green{(std::uint8_t)(y + state->greenOffset)};

            *pixel++ = ((green << 8) | blue);
        }

        row += buffer->pitch;
    }
}
void outputSine(GameState *state, GameSoundBuffer *buffer) {
    std::int16_t toneVolume{6000};
    std::int32_t wavePeriod{buffer->samplesPerSec / state->toneHz};

    std::int16_t *sampleOut{buffer->memory};
    for (int sampleIndex = 0; sampleIndex < buffer->samplesNeeded;
         ++sampleIndex) {
        float sineValue{
            std::sinf(2 * std::numbers::pi * (float)(buffer->t) / wavePeriod)};
        std::int16_t sampleValue{(short)(sineValue * toneVolume)};
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;

        buffer->t += 1;
        if (buffer->t > wavePeriod) {
            buffer->t = 1;
        }
    }
}
void updateGame(GameMemory *memory, GameInput *input) {
    GameState *gameState;
    gameState = (GameState *)memory->permanentStorage;

    if (!memory->isInitialized) {
        gameState->toneHz = 256;
        gameState->blueOffset = 0;
        gameState->greenOffset = 0;
        memory->isInitialized = true;
    }

    if (input->speedUp.endedDown) {
        gameState->greenOffset += 1;
    }
    if (input->strafeLeft.endedDown) {
        gameState->blueOffset += 1;
    }
    if (input->speedDown.endedDown) {
        gameState->greenOffset -= 1;
    }
    if (input->strafeRight.endedDown) {
        gameState->blueOffset -= 1;
    }
}
void fillBuffers(GameMemory *memory, GameOffscreenBuffer *offscreenBuffer,
                 GameSoundBuffer *soundBuffer) {
    GameState *gameState;
    gameState = (GameState *)memory->permanentStorage;

    renderWeirdGradient(gameState, offscreenBuffer);
    if (soundBuffer->enabled) {
        outputSine(gameState, soundBuffer);
    }
}
} // namespace Handmade
