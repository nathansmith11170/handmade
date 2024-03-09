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
module Handmade;
import std;

std::uint64_t Time64ToU64(Time64 t) {
    return (std::uint64_t)(t.wholeSeconds) << 32 | t.fraction;
}

Time64 Time64AddFloat(Time64 t, float addend) {
    auto addendU64{(std::uint64_t)(addend * std::pow(2, 32) + 0.5f)};
    auto res{Time64ToU64(t) + addendU64};
    return Time64{(std::uint32_t)(res >> 32),
                  (std::uint32_t)(res & 0xFFFFFFFF)};
}

void renderWeirdGradient(GameState *state, GameOffscreenBuffer *buffer) {
    std::int32_t offset{0};
    for (int y : std::ranges::iota_view{0, buffer->height}) {
        for (int x : std::ranges::iota_view{0, buffer->width}) {
            auto blue{(std::uint8_t)(x + state->blueOffset)};
            auto green{(std::uint8_t)(y + state->greenOffset)};
            auto bytes{(std::uint32_t)(blue) << 8 | green};

            buffer->memory[offset] = bytes;
            offset++;
        }
    }
}

void outputSine(GameState *state, GameSoundBuffer *buffer) {
    std::int16_t toneVolume{6000};
    std::int32_t wavePeriod{buffer->samplesPerSec / state->toneHz};

    std::int32_t outIndex{0};
    for (int sampleIndex : std::ranges::iota_view{0, buffer->samplesNeeded}) {
        auto sineValue{
            std::sinf(2 * std::numbers::pi * (float)(buffer->t) / wavePeriod)};
        auto sampleValue{(std::int16_t)(sineValue * toneVolume)};

        buffer->memory[outIndex] = sampleValue;
        outIndex++;
        buffer->memory[outIndex] = sampleValue;
        outIndex++;

        buffer->t += 1;
        if (buffer->t > wavePeriod) {
            buffer->t = 1;
        }
    }
}

void updateGame(GameMemory *memory, GameInput *input) {
    auto gameState = (GameState *)memory->permanentStorage.data();

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
    auto gameState = (GameState *)memory->permanentStorage.data();

    renderWeirdGradient(gameState, offscreenBuffer);
    if (soundBuffer->enabled) {
        outputSine(gameState, soundBuffer);
    }
}
