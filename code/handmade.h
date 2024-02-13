#if !defined(HANDMADE_H)
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
#include <stdint.h>
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef int bool32;

#define internal static
#define local_persist static
#define global_variable static

#define ArraySize(array) (sizeof(array) / sizeof((array)[0]))

#define Kilobytes(n) ((n) * 1024LL)
#define Megabytes(n) ((Kilobytes(n)) * 1024LL)
#define Gigabytes(n) ((Megabytes(n)) * 1024LL)

#define PI32 (3.14159265359f)

struct GameOffscreenBuffer {
  void *memory;
  u32 width;
  u32 height;
  u32 pitch;
};

struct GameSoundBuffer {
  void *memory;
  u32 samples_per_sec;
  u32 running_sample_index;
  u32 bytes_per_sample;
  u32 samples_needed;
};

// Note(Nathan) this might not be the final form of input management, because we might want to do separate thread or
// otherwise handle timing better, etc
struct GameKeyState {
  int half_transition_count;
  bool32 ended_down;
};

struct GameKeyboardInput {
  GameKeyState speed_up;
  GameKeyState speed_down;
  GameKeyState strafe_left;
  GameKeyState strafe_right;
};

struct GameInput {
  GameKeyboardInput inputs[16];
};

struct GameMemory {
  bool32 is_initialized;
  u64 permanent_store_size;
  void *permanent_store;
  u64 transient_storage_size;
  void *transient_storage;
};

struct DebugReadFileResult {
  u64 contents_size;
  void *contents;
};

internal DebugReadFileResult DEBUG_platform_read_entire_file(const char *file_name);
internal void DEBUG_platform_free_file_memory(void *memory);
internal bool32 DEBUG_platform_write_entire_file(const char *file_name, u64 memory_size, const void *memory);

internal void game_update_and_render(GameMemory *memory, GameOffscreenBuffer *buffer, GameSoundBuffer *sound_buffer,
                                     GameKeyboardInput *inputs);

#define HANDMADE_H
#endif
