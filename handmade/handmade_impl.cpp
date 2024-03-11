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
#include "handmade.hpp"
#include <cmath>
#include <cstdint>
#include <numbers>
#include <ranges>

uint64_t Time64ToU64(Time64 t) {
    return static_cast<uint64_t>(t.wholeSeconds) << 32 | t.fraction;
}

Time64 Time64AddFloat(Time64 t, float addend) {
    auto addendU64{static_cast<uint64_t>(addend * pow(2, 32) + 0.5f)};
    auto res{Time64ToU64(t) + addendU64};
    return Time64{static_cast<uint32_t>(res >> 32),
                  static_cast<uint32_t>(res & 0xFFFFFFFF)};
}

void renderWeirdGradient(GameState *state, GameOffscreenBuffer *buffer) {
    int offset{0};
    for (int y : std::ranges::iota_view{0, buffer->height}) {
        for (int x : std::ranges::iota_view{0, buffer->width}) {
            auto blue{static_cast<uint8_t>(x + state->blueOffset)};
            auto green{static_cast<uint8_t>(y + state->greenOffset)};

            auto bytes{static_cast<uint32_t>(blue) << 8 | green};

            memcpy(&(buffer->memory[offset]), &bytes, 4);
            offset += 4;
        }
    }
}

void outputSine(GameState *state, GameSoundBuffer *buffer) {
    int16_t toneVolume{6000};
    int wavePeriod{buffer->samplesPerSec / state->toneHz};

    int outIndex{0};
    for (int sampleIndex : std::ranges::iota_view{0, buffer->samplesNeeded}) {
        auto sineValue{sinf(2 * std::numbers::pi *
                            static_cast<float>(buffer->t) / wavePeriod)};
        auto sampleValue{static_cast<int16_t>(sineValue * toneVolume)};

        memcpy(&(buffer->memory[outIndex]), &sampleValue, 2);
        outIndex += 2;
        memcpy(&(buffer->memory[outIndex]), &sampleValue, 2);
        outIndex += 2;

        buffer->t += 1;
        if (buffer->t > wavePeriod) {
            buffer->t = 1;
        }
    }
}

void updateGame(GameMemory *memory, GameInput *input) {
    auto gameState =
        reinterpret_cast<GameState *>(memory->permanentStorage.data());

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
    auto gameState =
        reinterpret_cast<GameState *>(memory->permanentStorage.data());

    renderWeirdGradient(gameState, offscreenBuffer);
    if (soundBuffer->enabled) {
        outputSine(gameState, soundBuffer);
    }
}
