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
#include "handmade.h"
#include <cmath>
#include <numbers>

static void renderWeirdGradient(GameState *state, GameOffscreenBuffer *buffer) {
  unsigned char *row = buffer->memory;
  for (int y = 0; y < buffer->height; ++y) {
    unsigned int *pixel = (unsigned int *)row;
    for (int x = 0; x < buffer->width; ++x) {
      unsigned char blue = (unsigned char)(x + state->blueOffset);
      unsigned char green = (unsigned char)(y + state->greenOffset);

      *pixel++ = ((green << 8) | blue);
    }

    row += buffer->pitch;
  }
}

static void outputSine(GameState *state, GameSoundBuffer *buffer) {
  short toneVolume = 6000;
  int wavePeriod = buffer->samplesPerSec / state->toneHz;

  short *sampleOut = buffer->memory;
  for (int sampleIndex = 0; sampleIndex < buffer->samplesNeeded;
       ++sampleIndex) {
    float sineValue =
        sinf(2 * std::numbers::pi * (float)(buffer->t) / wavePeriod);
    short sampleValue = (short)(sineValue * toneVolume);
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
