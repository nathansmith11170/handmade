#include "handmade.h"
#include <math.h> // TODO(Nathan) Remove dependency, implement sine

typedef struct GameState {
  u32 blue_offset;
  u32 green_offset;
  u32 tone_hz;
} GameState;

void render_weird_gradient(GameState *game_state, GameOffscreenBuffer *buffer) {
  u8 *row_ptr = (u8 *)(buffer->memory);
  for (u32 y = 0; y < buffer->height; ++y) {
    u32 *pixel_ptr = (u32 *)(row_ptr);
    for (u32 x = 0; x < buffer->width; ++x) {
      u8 blue = (u8)(x + game_state->blue_offset);
      u8 green = (u8)(y + game_state->green_offset);

      *pixel_ptr = ((u32)(green) << 8) | (u32)(blue);
      pixel_ptr++;
    }
    row_ptr += buffer->pitch;
  }
}

void output_game_sound(GameState *game_state, GameSoundBuffer *sound_buffer) {
  i16 tone_volume = 12000;
  f32 wave_period = (f32)(sound_buffer->samples_per_sec) / (f32)(game_state->tone_hz);
  if (sound_buffer->samples_needed) {
    i16 *sample_out = (i16 *)sound_buffer->memory;
    for (u32 sample_i = 0; sample_i < sound_buffer->samples_needed; ++sample_i) {
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

void game_update_and_render(GameMemory *memory, GameOffscreenBuffer *buffer, GameSoundBuffer *sound_buffer,
                            GameKeyboardInput *input) {
  GameState *game_state = (GameState *)memory->permanent_store;
  if (!(memory->is_initialized)) {
    game_state->tone_hz = 256;
    memory->is_initialized = true;
  }

  // Controls?
  // Increase Thrust, Decrease Thrust, strafe left, strafe right
  //    -default mapping wasd, caps lock to toggle follow mouse which changes a & d to strafe and rotate nose toward
  //    mouse

  // targeting
  //    -Add a new locked target key (left click on target?)
  //    -Unlock target key (right click on UI?)
  //    -Lock nearest (maybe E (shift + e)? or just lock the nearest target if there are none and 'cycle next target' is
  //    input)
  //    -Change active target keys (for weapons or support items) default q, e, and only cycle thru hostile
  //    targets shift to cycle through allied targets only

  // quickbinds for weapons - fire once, toggle autofire, toggle autofire mode (point defense, active target, etc)
  //    -4 weapon groups, 1-4? shift + 1 to toggle autofire for group 1, alt + 1 to change fire mode of group 1

  // defense actions - pulse shield, active countermeasures, activate energized armor, etc
  //    -basically have 3 generic binds

  // Burst jets key
  //    -These jets give an additional kick in the 'forward' direction, double tap increase thrust key

  // Reverse jets
  //    -These jets give a kick in the direction opposite current bearing, double tap decrease thrust key
  if (input->speed_up.ended_down) {
    (game_state->green_offset)++;
  }
  if (input->speed_down.ended_down) {
    (game_state->green_offset--);
  }
  if (input->strafe_left.ended_down) {
    (game_state->blue_offset)++;
  }
  if (input->strafe_right.ended_down) {
    (game_state->blue_offset)--;
  }

  // TODO(Nathan) handle timestamping sound samples and offsets for error recovering
  output_game_sound(game_state, sound_buffer);
  render_weird_gradient(game_state, buffer);
}
