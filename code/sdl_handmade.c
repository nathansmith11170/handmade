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

#include "SDL.h"
#include "SDL_audio.h"
#include "SDL_blendmode.h"
#include "SDL_error.h"
#include "SDL_pixels.h"
#include "SDL_render.h"
#include "SDL_surface.h"
#include "SDL_timer.h"
#include "SDL_video.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
const char title[] = "Handmade";
const int text_size = 12;

typedef struct SdlContext {
  int w;
  int h;
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Surface *backbuffer;
  SDL_Texture *screen;
  SDL_AudioSpec AudioSettings;
  SDL_AudioDeviceID audio_device_id;
} SdlContext;

typedef struct Game {
  SdlContext sdl;
  bool should_quit;
  u8 x_offset;
  u8 y_offset;
} Game;

void set_sdl_context(SdlContext *sdlc, int w, int h, const char title[]) {
  int result_code = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  atexit(SDL_Quit);
  if (result_code < 0) {
    const char *err = SDL_GetError();
    printf("SDL error in SDL_Init: %s", err);
    exit(1);
  }

  u8 bits_per_pixel = 32;
  sdlc->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_RESIZABLE);
  sdlc->renderer = SDL_CreateRenderer(sdlc->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  sdlc->w = w;
  sdlc->h = h;
  sdlc->backbuffer = SDL_CreateRGBSurface(0, w, h, bits_per_pixel, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
  SDL_SetSurfaceRLE(sdlc->backbuffer, 0);
  SDL_SetSurfaceBlendMode(sdlc->backbuffer, SDL_BLENDMODE_NONE);
  sdlc->screen = SDL_CreateTexture(sdlc->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
}

void resize_screen(Game *g, int w, int h) {
  if (g->sdl.screen) {
    SDL_DestroyTexture(g->sdl.screen);
  }
  if (g->sdl.backbuffer) {
    SDL_FreeSurface(g->sdl.backbuffer);
  }
  u8 bits_per_pixel = 32;
  g->sdl.screen = SDL_CreateTexture(g->sdl.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
  g->sdl.backbuffer = SDL_CreateRGBSurface(0, w, h, bits_per_pixel, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
  SDL_SetSurfaceBlendMode(g->sdl.backbuffer, SDL_BLENDMODE_NONE);
  SDL_SetSurfaceRLE(g->sdl.backbuffer, 0);
}

void draw_begin(Game *g) { SDL_RenderClear(g->sdl.renderer); }

void draw_weird_gradient(Game *g) {
  u8 *row_ptr = (u8 *)(g->sdl.backbuffer->pixels);
  for (int y = 0; y < g->sdl.backbuffer->h; ++y) {
    u32 *pixel_ptr = (u32 *)(row_ptr);
    for (int x = 0; x < g->sdl.backbuffer->w; ++x) {
      u8 blue = (u8)(x + g->x_offset);
      u8 green = (u8)(y + g->y_offset);

      *pixel_ptr = ((u32)(green) << 8) | (u32)(blue);
      pixel_ptr++;
    }
    row_ptr += g->sdl.backbuffer->pitch;
  }

  SDL_Surface *screen_buffer;
  SDL_LockTextureToSurface(g->sdl.screen, nullptr, &screen_buffer);
  SDL_BlitSurface(g->sdl.backbuffer, nullptr, screen_buffer, nullptr);
  SDL_UnlockTexture(g->sdl.screen);
  SDL_RenderCopy(g->sdl.renderer, g->sdl.screen, nullptr, nullptr);
}

void draw_end(Game *g) { SDL_RenderPresent(g->sdl.renderer); }

void init_sound_device(Game *g, i32 samplesPerSecond) {
  g->sdl.AudioSettings.freq = samplesPerSecond;
  g->sdl.AudioSettings.format = AUDIO_S16LSB;
  g->sdl.AudioSettings.channels = 2;
  g->sdl.AudioSettings.samples = 1024;
  g->sdl.AudioSettings.callback = nullptr;

  g->sdl.audio_device_id =
      SDL_OpenAudioDevice(nullptr, false, &g->sdl.AudioSettings, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE);
  if (g->sdl.audio_device_id == 0) {
    const char *err = SDL_GetError();
    printf("Error opening audio device: %s\n", err);
  }

  printf("Initialised an Audio device at frequency %d Hz, %d Channels, buffer "
         "size %d\n",
         g->sdl.AudioSettings.freq, g->sdl.AudioSettings.channels, g->sdl.AudioSettings.samples);

  if (g->sdl.AudioSettings.format != AUDIO_S16LSB) {
    printf("Oops! We didn't get AUDIO_S16LSB as our sample format!\n");
    SDL_CloseAudioDevice(g->sdl.audio_device_id);
  }
}

void handle_event(Game *g, SDL_Event *event) {
  switch (event->type) {
  case SDL_QUIT: {
    printf("SDL_QUIT\n");
    g->should_quit = true;
  } break;

  case SDL_KEYDOWN:
  case SDL_KEYUP: {
    SDL_Keycode keyCode = event->key.keysym.sym;
    // bool was_down = false;
    // if (event->key.state == SDL_RELEASED) {
    //   was_down = true;
    // } else if (event->key.repeat != 0) {
    //   was_down = true;
    // }

    if (keyCode == SDLK_w && event->key.state == SDL_PRESSED) {
      g->y_offset += 2;
    }
  } break;

  case SDL_WINDOWEVENT: {
    switch (event->window.event) {
    case SDL_WINDOWEVENT_SIZE_CHANGED: {
      printf("SDL_WINDOWEVENT_RESIZED (%d, %d)\n", event->window.data1, event->window.data2);
      resize_screen(g, event->window.data1, event->window.data2);
    } break;

    case SDL_WINDOWEVENT_FOCUS_GAINED: {
      printf("SDL_WINDOWEVENT_FOCUS_GAINED\n");
    } break;
    }
  } break;
  }
}

int main() {
  Game game = {};
  set_sdl_context(&game.sdl, 1024, 728, title);
  game.should_quit = false;

  // NOTE: Sound test
  u32 samples_per_sec = 44100;
  u32 tone_hz = 256;
  i16 tone_volume = 3000;
  u32 running_sample_index = 0;
  f32 wave_period = (f32)(samples_per_sec) / (f32)(tone_hz);
  u32 bytes_per_sample = sizeof(i16) * 2;

  init_sound_device(&game, (int)(samples_per_sec));
  bool is_sound_paused = true;
  u64 last_mark = SDL_GetPerformanceCounter();
  u32 frames = 0;
  while (!game.should_quit) {
    frames++;

    draw_begin(&game);
    draw_weird_gradient(&game);
    draw_end(&game);

    ++game.x_offset;

    // Sound output test
    u32 target_queue_bytes = (u32)(samples_per_sec * bytes_per_sample);
    u32 bytes_to_write = target_queue_bytes - SDL_GetQueuedAudioSize(game.sdl.audio_device_id);
    if (bytes_to_write) {
      void *sound_buffer = malloc(bytes_to_write);
      i16 *sample_out = (i16 *)sound_buffer;
      u32 sample_count = bytes_to_write / bytes_per_sample;
      for (u32 sample_i = 0; sample_i < sample_count; ++sample_i) {
        f32 t = 2.0f * pi * (f32)(running_sample_index) / wave_period;
        f32 sine_value = sinf(t);
        i16 sample_val = (i16)(sine_value * tone_volume);
        *sample_out++ = sample_val;
        *sample_out++ = sample_val;
        ++running_sample_index;
      }
      SDL_QueueAudio(game.sdl.audio_device_id, sound_buffer, bytes_to_write);
      free(sound_buffer);
    }

    if (is_sound_paused) {
      is_sound_paused = false;
      SDL_PauseAudioDevice(game.sdl.audio_device_id, is_sound_paused);
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      handle_event(&game, &event);
    }
    u64 current = SDL_GetPerformanceCounter();
    if ((f32)(current - last_mark) / (f32)SDL_GetPerformanceFrequency() > 1.0f) {
      printf("FPS: %d\n", frames);
      last_mark = current;
      frames = 0;
    };
  }

  SDL_CloseAudioDevice(game.sdl.audio_device_id);
  exit(0);
}
