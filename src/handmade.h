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
#include <cstdint>
#if !defined(HANDMADE_H)
struct DebugReadFileResult {
  void *data;
  int fileSize;
};

// static DebugReadFileResult debugPlatformReadEntireFile(const char *fileName);
// static void debugPlatformFreeFileMemory(void *memory);
// static bool debugPlatformWriteEntireFile(const char *fileName, int fileSize,
//                                          void *memory);

struct GameOffscreenBuffer {
  uint8_t *memory;
  int32_t height, width, pitch;
};

struct GameSoundBuffer {
  int16_t *memory;
  int32_t samplesPerSec;
  int32_t t;
  int32_t bytesPerSample;
  int32_t samplesNeeded;
  bool enabled;
};

struct GameButtonState {
  int32_t halfTransitionCount;
  bool endedDown;
};

struct GameInput {
  GameButtonState speedUp;
  GameButtonState speedDown;
  GameButtonState strafeLeft;
  GameButtonState strafeRight;
};

struct GameState {
  int32_t blueOffset;
  int32_t greenOffset;
  int32_t toneHz;
};

struct GameMemory {
  bool isInitialized;

  uint64_t permanentStorageSize;
  unsigned char *permanentStorage;

  uint64_t transientStorageSize;
  unsigned char *transientStorage;
};

#define HANDMADE_H
#endif
