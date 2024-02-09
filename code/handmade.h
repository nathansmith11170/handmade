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

const f32 pi = 3.1415927f;

typedef struct GameOffscreenBuffer {
  void *memory;
  u32 width;
  u32 height;
  u32 pitch;
} GameOffscreenBuffer;

typedef struct GameSoundBuffer {
  void *memory;
  u32 samples_per_sec;
  u32 running_sample_index;
  u32 bytes_per_sample;
} GameSoundBuffer;

void game_update_and_render(GameOffscreenBuffer *buffer, GameSoundBuffer *sound_buffer, u32 samples_count_to_output,
                            u32 blue_offset, u32 green_offset);

#define HANDMADE_H
#endif
