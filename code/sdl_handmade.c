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

typedef struct GameInterface {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Surface *backbuffer;
  SDL_Texture *screen;
  SDL_AudioSpec AudioSettings;
  SDL_AudioDeviceID audio_device_id;
  GameSoundBuffer sound_buffer;
  GameMemory game_memory;
  GameKeyboardInput inputs;
  bool32 should_quit;
} GameInterface;

void set_sdl_context(GameInterface *sdlc, int w, int h, const char title[]) {
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
  sdlc->backbuffer = SDL_CreateRGBSurface(0, w, h, bits_per_pixel, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
  SDL_SetSurfaceRLE(sdlc->backbuffer, 0);
  SDL_SetSurfaceBlendMode(sdlc->backbuffer, SDL_BLENDMODE_NONE);
  sdlc->screen = SDL_CreateTexture(sdlc->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
}

void reserve_game_memory(GameInterface *g) {
  g->game_memory.permanent_store_size = Megabytes((u64)64);
  g->game_memory.permanent_store = malloc(g->game_memory.permanent_store_size);
  g->game_memory.transient_storage_size = Gigabytes((u64)4);
  g->game_memory.transient_storage = malloc(g->game_memory.transient_storage_size);
  g->game_memory.is_initialized = false;

  if (!(g->game_memory.permanent_store) || !(g->game_memory.transient_storage)) {
    printf("Failed to allocate game memory, exiting.");
    exit(1);
  }
}

void resize_screen(GameInterface *g, int w, int h) {
  if (g->screen) {
    SDL_DestroyTexture(g->screen);
  }
  if (g->backbuffer) {
    SDL_FreeSurface(g->backbuffer);
  }
  u8 bits_per_pixel = 32;
  g->screen = SDL_CreateTexture(g->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
  g->backbuffer = SDL_CreateRGBSurface(0, w, h, bits_per_pixel, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
  SDL_SetSurfaceBlendMode(g->backbuffer, SDL_BLENDMODE_NONE);
  SDL_SetSurfaceRLE(g->backbuffer, 0);
}

void draw_begin(GameInterface *g) { SDL_RenderClear(g->renderer); }

void draw_game_buffer_queue_sound(GameInterface *g) {
  GameOffscreenBuffer buffer;
  buffer.memory = g->backbuffer->pixels;
  buffer.width = (u32)g->backbuffer->w;
  buffer.height = (u32)g->backbuffer->h;
  buffer.pitch = (u32)g->backbuffer->pitch;

  // Sound output test
  u32 target_queue_bytes = (u32)(g->sound_buffer.samples_per_sec * g->sound_buffer.bytes_per_sample);
  u32 bytes_to_write = target_queue_bytes - SDL_GetQueuedAudioSize(g->audio_device_id);
  g->sound_buffer.samples_needed = bytes_to_write / g->sound_buffer.bytes_per_sample;
  game_update_and_render(&(g->game_memory), &buffer, &(g->sound_buffer), &(g->inputs));

  SDL_QueueAudio(g->audio_device_id, g->sound_buffer.memory, bytes_to_write);

  SDL_Surface *screen_buffer;
  SDL_LockTextureToSurface(g->screen, NULL, &screen_buffer);
  SDL_BlitSurface(g->backbuffer, NULL, screen_buffer, NULL);
  SDL_UnlockTexture(g->screen);
  SDL_RenderCopy(g->renderer, g->screen, NULL, NULL);
}

void draw_end(GameInterface *g) { SDL_RenderPresent(g->renderer); }

void init_sound_device(GameInterface *g) {
  // NOTE: Sound test
  g->sound_buffer.samples_per_sec = 44100;
  g->sound_buffer.running_sample_index = 0;

  g->AudioSettings.freq = (int)g->sound_buffer.samples_per_sec;
  g->AudioSettings.format = AUDIO_S16LSB;
  g->AudioSettings.channels = 2;
  g->AudioSettings.samples = 1024;
  g->AudioSettings.callback = NULL;

  g->audio_device_id = SDL_OpenAudioDevice(NULL, false, &g->AudioSettings, NULL, SDL_AUDIO_ALLOW_ANY_CHANGE);
  if (g->audio_device_id == 0) {
    const char *err = SDL_GetError();
    printf("Error opening audio device: %s\n", err);
  }

  g->sound_buffer.bytes_per_sample = sizeof(i16) * g->AudioSettings.channels;
  g->sound_buffer.memory = malloc(g->sound_buffer.samples_per_sec * g->sound_buffer.bytes_per_sample);
  printf("Initialised an Audio device at frequency %d Hz, %d Channels, buffer "
         "size %d\n",
         g->AudioSettings.freq, g->AudioSettings.channels, g->AudioSettings.samples);

  if (g->AudioSettings.format != AUDIO_S16LSB) {
    printf("Oops! We didn't get AUDIO_S16LSB as our sample format!\n");
    SDL_CloseAudioDevice(g->audio_device_id);
  }
}

void handle_event(GameInterface *g, SDL_Event *event) {
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

    if (keyCode == SDLK_w && event->key.state == SDL_RELEASED) {
      g->inputs.speed_up.ended_down = false;
    }
    if (keyCode == SDLK_a && event->key.state == SDL_RELEASED) {
      g->inputs.strafe_left.ended_down = false;
    }
    if (keyCode == SDLK_s && event->key.state == SDL_RELEASED) {
      g->inputs.speed_down.ended_down = false;
    }
    if (keyCode == SDLK_d && event->key.state == SDL_RELEASED) {
      g->inputs.strafe_right.ended_down = false;
    }

    if (keyCode == SDLK_w && event->key.state == SDL_PRESSED) {
      g->inputs.speed_up.ended_down = true;
    }
    if (keyCode == SDLK_a && event->key.state == SDL_PRESSED) {
      g->inputs.strafe_left.ended_down = true;
    }
    if (keyCode == SDLK_s && event->key.state == SDL_PRESSED) {
      g->inputs.speed_down.ended_down = true;
    }
    if (keyCode == SDLK_d && event->key.state == SDL_PRESSED) {
      g->inputs.strafe_right.ended_down = true;
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

int main(void) {
  GameInterface game;
  set_sdl_context(&game, 1024, 728, title);
  reserve_game_memory(&game);
  game.should_quit = false;

  game.inputs.speed_down.ended_down = false;
  game.inputs.speed_up.ended_down = false;
  game.inputs.strafe_left.ended_down = false;
  game.inputs.strafe_right.ended_down = false;

  init_sound_device(&game);
  bool32 is_sound_paused = true;
  u64 last_mark = SDL_GetPerformanceCounter();
  u32 frames = 0;
  while (!game.should_quit) {
    frames++;

    draw_begin(&game);
    draw_game_buffer_queue_sound(&game);
    draw_end(&game);

    if (is_sound_paused) {
      is_sound_paused = false;
      SDL_PauseAudioDevice(game.audio_device_id, is_sound_paused);
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
  free(game.sound_buffer.memory);
  SDL_CloseAudioDevice(game.audio_device_id);
  exit(0);
}
