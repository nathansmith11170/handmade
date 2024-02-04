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
global_variable SDL_Texture *Texture {};
global_variable int BitmapWidth {};
global_variable int BitmapHeight {};
global_variable std::vector<uint8_t> BitmapMemory {};
global_variable int BytesPerPixel {4};
global_variable long CountedFrames {};
global_variable std::chrono::steady_clock::time_point StartTime {};
global_variable TTF_Font *Sans {};
global_variable std::stringstream FpsText {};
global_variable SDL_Color White {255, 255, 255, 255};
global_variable SDL_Texture *FpsMessageTexture {};

void handleEvent(SDL_Event *event);
internal void outputSDLError(std::string caller);
internal void outputTTFError(std::string caller);
internal SDL_Texture *getFPSTexture(SDL_Renderer *renderer, SDL_Rect *message_rect);
internal void SDLResizeTexture(SDL_Renderer *renderer, int width, int height);
internal void SDLUpdateWindow(SDL_Window *Window, SDL_Renderer *Renderer);

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
  if (Texture != nullptr) {
    SDL_DestroyTexture(Texture);
  }
  BitmapMemory.clear();

  Texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
  BitmapWidth = width;
  BitmapHeight = height;
  BitmapMemory.reserve(width * height * BytesPerPixel);
  std::cout << "Allocated bitmap size: " << BitmapMemory.size() * sizeof(uint8_t) << '\n';
}

internal void SDLUpdateWindow(SDL_Window *window, SDL_Renderer *renderer) {
  uint8_t *texture_data = NULL;
  int texture_pitch = 0;

  SDL_LockTexture(Texture, 0, (void **)&texture_data, &texture_pitch);
  memcpy(texture_data, BitmapMemory.data(), BitmapWidth * BitmapHeight * BytesPerPixel);
  SDL_UnlockTexture(Texture);

  SDL_RenderCopy(renderer, Texture, NULL, NULL);

  SDL_Rect fps_rect;
  fps_rect.x = 0;
  fps_rect.y = 0;
  FpsMessageTexture = getFPSTexture(renderer, &fps_rect);
  if (FpsMessageTexture != nullptr) {
    SDL_RenderCopy(renderer, FpsMessageTexture, NULL, &fps_rect);
  }
  SDL_RenderPresent(renderer);
}

internal void renderWeirdGradient(int x_offset, int y_offset) {
  int pitch = BitmapWidth * BytesPerPixel;

  int row_start = 0;
  for (int y {0}; y < BitmapHeight; ++y) {
    for (int x {0}; x < BitmapWidth; ++x) {
      BitmapMemory[row_start + BytesPerPixel * x + 0] = (uint8_t)(x + x_offset);

      BitmapMemory[row_start + BytesPerPixel * x + 1] = (uint8_t)(y);

      BitmapMemory[row_start + BytesPerPixel * x + 2] = 0;

      BitmapMemory[row_start + BytesPerPixel * x + 3] = 0;
    }

    row_start += pitch;
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
      int width {};
      int height {};
      SDL_GetWindowSize(SDL_GetWindowFromID(event->window.windowID), &width, &height);
      SDLResizeTexture(SDL_GetRenderer(SDL_GetWindowFromID(event->window.windowID)), width, height);
      std::cout << "SDL_WINDOWEVENT_RESIZED ( " << event->window.data1 << "," << event->window.data2 << ")\n";
    } break;

    case SDL_WINDOWEVENT_FOCUS_GAINED: {
      printf("SDL_WINDOWEVENT_FOCUS_GAINED\n");
    } break;

    case SDL_WINDOWEVENT_EXPOSED: {
      SDL_Window *window = SDL_GetWindowFromID(event->window.windowID);
      SDL_Renderer *renderer = SDL_GetRenderer(window);
      SDLUpdateWindow(window, renderer);
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

  int width {};
  int height {};
  int x_offset {0};
  int y_offset {0};
  SDL_GetWindowSize(window, &width, &height);
  SDLResizeTexture(renderer, width, height);
  while (Running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      handleEvent(&event);
    }
    renderWeirdGradient(x_offset, y_offset);
    SDLUpdateWindow(window, renderer);

    ++x_offset;
    y_offset += 2;
    ++CountedFrames;
  }

  return 0;
}
