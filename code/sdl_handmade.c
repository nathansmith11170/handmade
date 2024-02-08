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

#include "SDL_blendmode.h"
#include "SDL_error.h"
#include "SDL_pixels.h"
#include "SDL_render.h"
#include "SDL_surface.h"
#include "SDL_video.h"
#include <SDL.h>
#include <SDL_ttf.h>
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

const char title[] = "Handmade";
const char font_name[] = "fonts/OpenSans-Regular.ttf";
const int text_size = 12;

typedef struct SdlContext {
  int w;
  int h;
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Surface *backbuffer;
  SDL_Texture *screen;
  TTF_Font *font;
} SdlContext;

typedef struct Game {
  SdlContext sdl;
  u32 fps;
  bool should_quit;
  u8 x_offset;
  u8 y_offset;
} Game;

void set_sdl_context(SdlContext *sdlc, int w, int h, const char title[]) {
  int result_code = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_VIDEO);
  atexit(SDL_Quit);
  if (result_code < 0) {
    const char *err = SDL_GetError();
    printf("SDL error in SDL_Init: %s", err);
    exit(1);
  }

  result_code = TTF_Init();
  atexit(TTF_Quit);
  if (result_code < 0) {
    const char *err = SDL_GetError();
    printf("TTF error in TTF_Init: %s", err);
    exit(1);
  }

  u8 bits_per_pixel = 32;
  SDL_CreateWindowAndRenderer(w, h, 0, &sdlc->window, &sdlc->renderer);
  SDL_SetWindowTitle(sdlc->window, title);
  SDL_SetWindowResizable(sdlc->window, true);
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

void draw_fps_text(Game *g) {
  size_t size = (size_t)snprintf(NULL, 0, "FPS: %u", g->fps);
  char *fpsText = (char *)malloc(size + 1);
  snprintf(fpsText, size + 1, "FPS: %u", g->fps);

  SDL_Color text_color = {255, 255, 255, 255};
  SDL_Surface *text_surface = TTF_RenderText_Solid(g->sdl.font, fpsText, text_color);
  SDL_Texture *text_texture = SDL_CreateTextureFromSurface(g->sdl.renderer, text_surface);
  int text_w = 0;
  int text_h = 0;
  SDL_QueryTexture(text_texture, nullptr, nullptr, &text_w, &text_h);
  SDL_Rect dst_rect = {0, 0, text_w, text_h};
  SDL_RenderCopy(g->sdl.renderer, text_texture, nullptr, &dst_rect);
  SDL_DestroyTexture(text_texture);
  SDL_FreeSurface(text_surface);
}

void draw_end(Game *g) { SDL_RenderPresent(g->sdl.renderer); }

SDL_AudioSpec AudioSettings = {};

void sdl_init_sound_device(i32 samplesPerSecond) {
  AudioSettings.freq = samplesPerSecond;
  AudioSettings.format = AUDIO_S16LSB;
  AudioSettings.channels = 2;
  AudioSettings.samples = 1024;
  AudioSettings.callback = nullptr;

  SDL_OpenAudio(&AudioSettings, nullptr);

  printf("Initialised an Audio device at frequency %d Hz, %d Channels, buffer "
         "size %d\n",
         AudioSettings.freq, AudioSettings.channels, AudioSettings.samples);

  if (AudioSettings.format != AUDIO_S16LSB) {
    printf("Oops! We didn't get AUDIO_S16LSB as our sample format!\n");
    SDL_CloseAudio();
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
  game.sdl.font = TTF_OpenFont(font_name, text_size);
  game.should_quit = false;

  // Start counting frames per second
  u32 countedFrames = 0;
  u64 lastTime = SDL_GetTicks64();

  // NOTE: Sound test
  u32 samples_per_sec = 44100;
  u32 tone_hz = 256;
  i16 tone_volume = 3000;
  u32 running_sample_index = 0;
  u32 square_wave_period = samples_per_sec / tone_hz;
  u32 half_square_wave_period = square_wave_period / 2;
  u32 bytes_per_sample = sizeof(i16) * 2;

  sdl_init_sound_device((int)(samples_per_sec));
  bool is_sound_paused = true;

  while (!game.should_quit) {
    u64 currentTime = SDL_GetTicks64();
    if (currentTime - lastTime >= 1000) {
      // Calculate frames per second
      game.fps = countedFrames;
      countedFrames = 0;
      lastTime = currentTime;
    }

    ++countedFrames;

    draw_begin(&game);
    draw_weird_gradient(&game);
    draw_fps_text(&game);
    draw_end(&game);

    ++game.x_offset;

    // Sound output test
    u32 target_queue_bytes = (u32)(samples_per_sec * bytes_per_sample);
    u32 bytes_to_write = target_queue_bytes - SDL_GetQueuedAudioSize(1);
    if (bytes_to_write) {
      void *sound_buffer = malloc(bytes_to_write);
      i16 *sample_out = (i16 *)sound_buffer;
      u32 sample_count = bytes_to_write / bytes_per_sample;
      for (u32 sample_i = 0; sample_i < sample_count; ++sample_i) {
        i16 sample_val = ((running_sample_index++ / half_square_wave_period) % 2) ? tone_volume : -tone_volume;
        *sample_out++ = sample_val;
        *sample_out++ = sample_val;
      }
      SDL_QueueAudio(1, sound_buffer, bytes_to_write);
      free(sound_buffer);
    }

    if (is_sound_paused) {
      is_sound_paused = false;
      SDL_PauseAudio(is_sound_paused);
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      handle_event(&game, &event);
    }
  }

  exit(0);
}
