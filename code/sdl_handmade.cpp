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
#include <chrono>
#include <iostream>
#include <sstream>
#include <vector>

#define internal static
#define local_persist static
#define global_variable static

global_variable bool Running {true};
global_variable long CountedFrames {};
global_variable std::chrono::steady_clock::time_point StartTime {};
global_variable const int BytesPerPixel {4};

global_variable TTF_Font *Sans {};
global_variable std::stringstream FpsText {};
global_variable SDL_Color White {255, 255, 255, 255};
global_variable SDL_Texture *FpsMessageTexture {};

struct sdl_offscreen_buffer {
  SDL_Texture *Texture;
  std::vector<uint8_t> Memory;
  int Width;
  int Height;
  int Pitch;
  // 32bit pixels in BGRX order
};

global_variable sdl_offscreen_buffer GlobalBackbuffer;

struct sdl_window_dimension {
  int Width;
  int Height;
};

void handleEvent(SDL_Event *event);
internal void outputSDLError(std::string caller);
internal void outputTTFError(std::string caller);
internal SDL_Texture *getFPSTexture(SDL_Renderer *renderer, SDL_Rect *message_rect);
internal void SDLResizeTexture(SDL_Renderer *renderer, int width, int height);
internal void SDLUpdateWindow(SDL_Renderer *Renderer);

sdl_window_dimension SDLGetWindowDimension(SDL_Window *window) {
  sdl_window_dimension result {};

  SDL_GetWindowSize(window, &result.Width, &result.Height);

  return result;
}

internal void outputSDLError(std::string caller) {
  const char *err {SDL_GetError()};
  std::cerr << "SDL error in " << caller << ": " << err << '\n';
}

internal void outputTTFError(std::string caller) {
  const char *err {TTF_GetError()};
  std::cerr << "TTF error in " << caller << ": " << err << '\n';
}

internal SDL_Texture *getFPSTexture(SDL_Renderer *renderer, SDL_Rect *message_rect) {
  if (FpsMessageTexture != nullptr) {
    SDL_DestroyTexture(FpsMessageTexture);
  }

  float float_seconds = std::chrono::duration<float> {std::chrono::steady_clock::now() - StartTime}.count();
  float avg_fps {static_cast<float>(CountedFrames) / float_seconds};

  FpsText.str("");
  FpsText << "FPS: " << avg_fps;

  SDL_Surface *message_surface {TTF_RenderText_Solid(Sans, FpsText.str().c_str(), White)};
  if (message_surface == nullptr) {
    outputTTFError("TTF_RenderText_Solid");
    return nullptr;
  }

  message_rect->w = message_surface->w;
  message_rect->h = message_surface->h;

  auto result = SDL_CreateTextureFromSurface(renderer, message_surface);
  if (result == nullptr) {
    outputSDLError("SDL_CreateTextureFromSurface");
    return nullptr;
  }
  SDL_FreeSurface(message_surface);
  return result;
}

internal void SDLResizeTexture(SDL_Renderer *renderer, int width, int height) {
  if (GlobalBackbuffer.Texture != nullptr) {
    SDL_DestroyTexture(GlobalBackbuffer.Texture);
  }
  GlobalBackbuffer.Memory.clear();

  GlobalBackbuffer.Texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
  GlobalBackbuffer.Width = width;
  GlobalBackbuffer.Height = height;
  GlobalBackbuffer.Pitch = width * BytesPerPixel;
  GlobalBackbuffer.Memory.reserve(static_cast<long unsigned int>(width * height) * BytesPerPixel);
  std::cout << "Allocated bitmap size: " << GlobalBackbuffer.Memory.capacity() * sizeof(uint8_t) << '\n';
}

internal void SDLUpdateWindow(SDL_Renderer *renderer) {
  uint8_t *texture_data = NULL;
  int texture_pitch = 0;

  SDL_LockTexture(GlobalBackbuffer.Texture, 0, (void **)&texture_data, &texture_pitch);
  memcpy(texture_data, GlobalBackbuffer.Memory.data(), GlobalBackbuffer.Memory.capacity() * sizeof(uint8_t));
  SDL_UnlockTexture(GlobalBackbuffer.Texture);

  SDL_RenderCopy(renderer, GlobalBackbuffer.Texture, NULL, NULL);

  SDL_Rect fps_rect;
  fps_rect.x = 0;
  fps_rect.y = 0;
  FpsMessageTexture = getFPSTexture(renderer, &fps_rect);
  if (FpsMessageTexture != nullptr) {
    SDL_RenderCopy(renderer, FpsMessageTexture, NULL, &fps_rect);
  }
  SDL_RenderPresent(renderer);
}

internal void renderWeirdGradient(long unsigned int x_offset, long unsigned int y_offset) {
  long unsigned int row_start = 0;
  for (int y {0}; y < GlobalBackbuffer.Height; ++y) {
    for (int x {0}; x < GlobalBackbuffer.Width * BytesPerPixel; x += BytesPerPixel) {
      GlobalBackbuffer.Memory[row_start + static_cast<long unsigned int>(x) + 0] =
          static_cast<uint8_t>(static_cast<long unsigned int>(x) + x_offset);

      GlobalBackbuffer.Memory[row_start + static_cast<long unsigned int>(x) + 1] =
          static_cast<uint8_t>(static_cast<long unsigned int>(y) + y_offset);

      GlobalBackbuffer.Memory[row_start + static_cast<long unsigned int>(x) + 2] = 0;

      GlobalBackbuffer.Memory[row_start + static_cast<long unsigned int>(x) + 3] = 0;
    }

    row_start += GlobalBackbuffer.Pitch;
  }
}

void handleEvent(SDL_Event *event) {
  switch (event->type) {
  case SDL_QUIT: {
    std::cout << "SDL_QUIT\n";
    Running = false;
  } break;

  case SDL_WINDOWEVENT: {
    switch (event->window.event) {
    case SDL_WINDOWEVENT_SIZE_CHANGED: {
      std::cout << "SDL_WINDOWEVENT_RESIZED ( " << event->window.data1 << "," << event->window.data2 << ")\n";
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
  int result_code {};

  result_code = SDL_Init(SDL_INIT_VIDEO);
  if (result_code < 0) {
    outputSDLError("SDL_Init");
    return 1;
  }

  result_code = TTF_Init();
  if (result_code < 0) {
    outputTTFError("TTF_Init");
    return 1;
  }

  auto window {
      SDL_CreateWindow("Handmade", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_RESIZABLE)};
  if (window == nullptr) {
    outputSDLError("SDL_CreateWindow");
    return 1;
  }

  auto renderer {SDL_CreateRenderer(window, -1, 0)};
  if (renderer == nullptr) {
    outputSDLError("SDL_CreateRenderer");
    return 1;
  }

  Sans = TTF_OpenFont("fonts/OpenSans-Regular.ttf", 11);
  if (Sans == nullptr) {
    outputTTFError("TTF_OpenFont");
    return 1;
  }

  // Start counting frames per second
  CountedFrames = 0;
  StartTime = std::chrono::steady_clock::now();

  long unsigned int x_offset {0};
  long unsigned int y_offset {0};
  auto window_dimensions {SDLGetWindowDimension(window)};
  SDLResizeTexture(renderer, window_dimensions.Width, window_dimensions.Height);
  while (Running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      handleEvent(&event);
    }
    renderWeirdGradient(x_offset, y_offset);
    SDLUpdateWindow(renderer);

    ++x_offset;
    y_offset += 2;
    ++CountedFrames;
  }

  return 0;
}
