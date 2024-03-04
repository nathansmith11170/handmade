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
  unsigned char *memory;
  int height, width, pitch;
};

struct GameSoundBuffer {
  short *memory;
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

  long long permanentStorageSize;
  unsigned char *permanentStorage;

  long long transientStorageSize;
  unsigned char *transientStorage;
};

#define HANDMADE_H
#endif
