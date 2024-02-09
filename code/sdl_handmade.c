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
#include "handmade.c"
#include "handmade.h"

#include "SDL.h"
#include "SDL_audio.h"
#include "SDL_blendmode.h"
#include "SDL_error.h"
#include "SDL_pixels.h"
#include "SDL_render.h"
#include "SDL_surface.h"
#include "SDL_timer.h"
#include "SDL_video.h"

#include <stdio.h>
#include <stdlib.h>

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
  GameSoundBuffer sound_buffer;
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

void draw_game_buffer_queue_sound(Game *g) {
  GameOffscreenBuffer buffer = {};
  buffer.memory = g->sdl.backbuffer->pixels;
  buffer.width = (u32)g->sdl.backbuffer->w;
  buffer.height = (u32)g->sdl.backbuffer->h;
  buffer.pitch = (u32)g->sdl.backbuffer->pitch;

  // Sound output test
  u32 target_queue_bytes = (u32)(g->sdl.sound_buffer.samples_per_sec * g->sdl.sound_buffer.bytes_per_sample);
  u32 bytes_to_write = target_queue_bytes - SDL_GetQueuedAudioSize(g->sdl.audio_device_id);
  u32 samples_needed = bytes_to_write / g->sdl.sound_buffer.bytes_per_sample;
  game_update_and_render(&buffer, &g->sdl.sound_buffer, samples_needed, g->x_offset, g->y_offset);

  SDL_QueueAudio(g->sdl.audio_device_id, g->sdl.sound_buffer.memory, bytes_to_write);

  SDL_Surface *screen_buffer;
  SDL_LockTextureToSurface(g->sdl.screen, nullptr, &screen_buffer);
  SDL_BlitSurface(g->sdl.backbuffer, nullptr, screen_buffer, nullptr);
  SDL_UnlockTexture(g->sdl.screen);
  SDL_RenderCopy(g->sdl.renderer, g->sdl.screen, nullptr, nullptr);
}

void draw_end(Game *g) { SDL_RenderPresent(g->sdl.renderer); }

void init_sound_device(Game *g) {
  // NOTE: Sound test
  g->sdl.sound_buffer.samples_per_sec = 44100;
  g->sdl.sound_buffer.running_sample_index = 0;

  g->sdl.AudioSettings.freq = (int)g->sdl.sound_buffer.samples_per_sec;
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

  g->sdl.sound_buffer.bytes_per_sample = sizeof(i16) * g->sdl.AudioSettings.channels;
  g->sdl.sound_buffer.memory = malloc(g->sdl.sound_buffer.samples_per_sec * g->sdl.sound_buffer.bytes_per_sample);
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

  init_sound_device(&game);
  bool is_sound_paused = true;
  u64 last_mark = SDL_GetPerformanceCounter();
  u32 frames = 0;
  while (!game.should_quit) {
    frames++;

    draw_begin(&game);
    draw_game_buffer_queue_sound(&game);
    draw_end(&game);

    ++game.x_offset;

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
  free(game.sdl.sound_buffer.memory);
  SDL_CloseAudioDevice(game.sdl.audio_device_id);
  exit(0);
}
