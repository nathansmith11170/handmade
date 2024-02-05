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

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

#define true 1
#define false 0

bool Running = true;
long CountedFrames = 0;
clock_t StartTime;
int BytesPerPixel = 4;

TTF_Font *Sans;
void *FpsText = NULL;
SDL_Color White = {255, 255, 255, 255};
struct SDL_Texture *FpsMessageTexture;

typedef struct sdl_offscreen_buffer {
  struct SDL_Texture *Texture;
  void *Memory;
  int Width;
  int Height;
  int Pitch;
  // 32bit pixels in BGRX order
} sdl_offscreen_buffer;

sdl_offscreen_buffer GlobalBackbuffer;

typedef struct sdl_window_dimension {
  int Width;
  int Height;
} sdl_window_dimension;

void handleEvent(SDL_Event *event, int *y_offset);
void outputSDLError(const char *caller);
void outputTTFError(const char *caller);
struct SDL_Texture *getFPSTexture(SDL_Renderer *renderer, SDL_Rect *message_rect);
void SDLResizeTexture(SDL_Renderer *renderer, int width, int height);
void SDLUpdateWindow(SDL_Renderer *Renderer);

sdl_window_dimension SDLGetWindowDimension(SDL_Window *window) {
  sdl_window_dimension result;

  SDL_GetWindowSize(window, &result.Width, &result.Height);

  return result;
}

void outputSDLError(const char *caller) {
  const char *err = SDL_GetError();
  printf("SDL error in %s: %s\n", caller, err);
}

void outputTTFError(const char *caller) {
  const char *err = TTF_GetError();
  printf("TTF error in %s: %s\n", caller, err);
}

SDL_Texture *getFPSTexture(SDL_Renderer *renderer, SDL_Rect *message_rect) {
  if (FpsMessageTexture) {
    SDL_DestroyTexture(FpsMessageTexture);
  }
  if (FpsText) {
    free(FpsText);
  }

  double float_seconds = (double)clock() / CLOCKS_PER_SEC - (double)StartTime;
  double avg_fps = (double)CountedFrames / float_seconds;

  size_t size = (size_t)snprintf(NULL, 0, "FPS: %.0f", avg_fps);
  FpsText = (char *)malloc(size + 1);
  snprintf((char *)(FpsText), size + 1, "FPS: %.0f", avg_fps);

  SDL_Surface *message_surface = TTF_RenderText_Solid(Sans, FpsText, White);
  if (!message_surface) {
    outputTTFError("TTF_RenderText_Solid");
    return NULL;
  }

  message_rect->w = message_surface->w;
  message_rect->h = message_surface->h;

  SDL_Texture *result = SDL_CreateTextureFromSurface(renderer, message_surface);
  if (!result) {
    outputSDLError("SDL_CreateTextureFromSurface");
    return NULL;
  }
  SDL_FreeSurface(message_surface);
  return result;
}

void SDLResizeTexture(SDL_Renderer *renderer, int width, int height) {
  if (GlobalBackbuffer.Texture) {
    SDL_DestroyTexture(GlobalBackbuffer.Texture);
  }
  if (GlobalBackbuffer.Memory) {
    munmap(GlobalBackbuffer.Memory, (size_t)(GlobalBackbuffer.Width * GlobalBackbuffer.Height * BytesPerPixel));
  }
  GlobalBackbuffer.Texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
  GlobalBackbuffer.Width = width;
  GlobalBackbuffer.Height = height;
  GlobalBackbuffer.Pitch = width * BytesPerPixel;
  GlobalBackbuffer.Memory = mmap(0, (size_t)(GlobalBackbuffer.Width * GlobalBackbuffer.Height * BytesPerPixel),
                                 PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

void SDLUpdateWindow(SDL_Renderer *renderer) {
  uint8_t *texture_data = NULL;
  int texture_pitch = 0;

  SDL_LockTexture(GlobalBackbuffer.Texture, 0, (void **)&texture_data, &texture_pitch);
  memcpy(texture_data, GlobalBackbuffer.Memory,
         (size_t)(GlobalBackbuffer.Height * GlobalBackbuffer.Pitch) * sizeof(uint8_t));
  SDL_UnlockTexture(GlobalBackbuffer.Texture);

  SDL_RenderCopy(renderer, GlobalBackbuffer.Texture, NULL, NULL);

  SDL_Rect fps_rect;
  fps_rect.x = 0;
  fps_rect.y = 0;
  FpsMessageTexture = getFPSTexture(renderer, &fps_rect);
  if (FpsMessageTexture) {
    SDL_RenderCopy(renderer, FpsMessageTexture, NULL, &fps_rect);
  }
  SDL_RenderPresent(renderer);
}

void renderWeirdGradient(int x_offset, int y_offset) {
  uint8_t *row = (uint8_t *)GlobalBackbuffer.Memory;
  for (int y = 0; y < GlobalBackbuffer.Height; ++y) {
    uint8_t *pixel = (uint8_t *)row;
    for (int x = 0; x < GlobalBackbuffer.Width; ++x) {
      *pixel = (uint8_t)(x + x_offset);
      ++pixel;

      *pixel = (uint8_t)(y + y_offset);
      ++pixel;

      *pixel = 0;
      ++pixel;

      *pixel = 0;
      ++pixel;
    }

    row += GlobalBackbuffer.Pitch;
  }
}

void handleEvent(SDL_Event *event, int *y_offset) {
  switch (event->type) {
  case SDL_QUIT: {
    printf("SDL_QUIT\n");
    Running = false;
  } break;

  case SDL_KEYDOWN:
  case SDL_KEYUP: {
    SDL_Keycode key_code = event->key.keysym.sym;
    // bool was_down = false;
    // if (event->key.state == SDL_RELEASED) {
    //   was_down = true;
    // } else if (event->key.repeat != 0) {
    //   was_down = true;
    // }

    if (key_code == SDLK_w && event->key.state == SDL_PRESSED) {
      *y_offset += 2;
    }
  } break;

  case SDL_WINDOWEVENT: {
    switch (event->window.event) {
    case SDL_WINDOWEVENT_SIZE_CHANGED: {
      printf("SDL_WINDOWEVENT_RESIZED (%d, %d)\n", event->window.data1, event->window.data2);
    } break;

    case SDL_WINDOWEVENT_FOCUS_GAINED: {
      printf("SDL_WINDOWEVENT_FOCUS_GAINED\n");
    } break;

    case SDL_WINDOWEVENT_EXPOSED: {
      SDL_Window *window = SDL_GetWindowFromID(event->window.windowID);
      SDL_Renderer *renderer = SDL_GetRenderer(window);
      SDLUpdateWindow(renderer);
    } break;
    }
  } break;
  }
}

int main() {
  int result_code = SDL_Init(SDL_INIT_VIDEO);
  if (result_code < 0) {
    outputSDLError("SDL_Init");
    return 1;
  }

  result_code = TTF_Init();
  if (result_code < 0) {
    outputTTFError("TTF_Init");
    return 1;
  }

  auto window =
      SDL_CreateWindow("Handmade", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1028, 720, SDL_WINDOW_RESIZABLE);
  if (!window) {
    outputSDLError("SDL_CreateWindow");
    return 1;
  }

  auto renderer = SDL_CreateRenderer(window, -1, 0);
  if (!renderer) {
    outputSDLError("SDL_CreateRenderer");
    return 1;
  }

  Sans = TTF_OpenFont("fonts/OpenSans-Regular.ttf", 11);
  if (!Sans) {
    outputTTFError("TTF_OpenFont");
    return 1;
  }

  GlobalBackbuffer.Texture = NULL;
  GlobalBackbuffer.Memory = NULL;
  GlobalBackbuffer.Height = 0;
  GlobalBackbuffer.Width = 0;
  GlobalBackbuffer.Pitch = 0;

  // Start counting frames per second
  CountedFrames = 0;
  StartTime = clock() / CLOCKS_PER_SEC;

  int x_offset = 0;
  int y_offset = 0;
  auto window_dimensions = SDLGetWindowDimension(window);
  SDLResizeTexture(renderer, window_dimensions.Width, window_dimensions.Height);
  while (Running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      handleEvent(&event, &y_offset);
    }
    renderWeirdGradient(x_offset, y_offset);
    SDLUpdateWindow(renderer);

    ++x_offset;
    ++CountedFrames;
  }

  return 0;
}
