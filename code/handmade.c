#include "handmade.h"
#include <math.h> // TODO(Nathan) Remove dependency, implement sine

void render_weird_gradient(GameOffscreenBuffer *buffer, u32 blue_offset, u32 green_offset) {
  u8 *row_ptr = (u8 *)(buffer->memory);
  for (u32 y = 0; y < buffer->height; ++y) {
    u32 *pixel_ptr = (u32 *)(row_ptr);
    for (u32 x = 0; x < buffer->width; ++x) {
      u8 blue = (u8)(x + blue_offset);
      u8 green = (u8)(y + green_offset);

      *pixel_ptr = ((u32)(green) << 8) | (u32)(blue);
      pixel_ptr++;
    }
    row_ptr += buffer->pitch;
  }
}

void output_game_sound(GameSoundBuffer *sound_buffer, u32 sample_count_to_output) {
  u32 tone_hz = 512;
  i16 tone_volume = 12000;
  f32 wave_period = (f32)(sound_buffer->samples_per_sec) / (f32)(tone_hz);
  if (sample_count_to_output) {
    i16 *sample_out = (i16 *)sound_buffer->memory;
    for (u32 sample_i = 0; sample_i < sample_count_to_output; ++sample_i) {
      f32 t = 2.0f * pi * (f32)(sound_buffer->running_sample_index) / wave_period;
      f32 sine_value = sinf(t);
      i16 sample_val = (i16)(sine_value * tone_volume);
      if (sound_buffer->bytes_per_sample == 4) {
        *sample_out++ = sample_val;
      }
      *sample_out++ = sample_val;
      ++(sound_buffer->running_sample_index);
      if ((f32)sound_buffer->running_sample_index > wave_period) {
        sound_buffer->running_sample_index = 1;
      }
    }
  }
}

void game_update_and_render(GameOffscreenBuffer *buffer, GameSoundBuffer *sound_buffer, u32 samples_needed,
                            u32 blue_offset, u32 green_offset) {
  // TODO(Nathan) handle timestamping sound samples and offsets for error recovering
  output_game_sound(sound_buffer, samples_needed);
  render_weird_gradient(buffer, blue_offset, green_offset);
}
