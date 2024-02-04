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

static bool running {true};
bool handleEvent(SDL_Event *Event);
void outputSDLErrorAndQuit(std::string caller);
void outputTTFErrorAndQuit(std::string caller);
SDL_Texture *getFPSAsTexture(std::chrono::steady_clock::time_point start, long counted_frames,
                             std::stringstream *time_text, SDL_Renderer *renderer, TTF_Font *font,
                             SDL_Color *text_color, SDL_Rect *message_rect);

int main() {
  int call_code {SDL_Init(SDL_INIT_VIDEO)};
  if (call_code < 0) {
    outputSDLErrorAndQuit("SDL_Init");
    return 1;
  }

  call_code = TTF_Init();
  if (call_code < 0) {
    outputTTFErrorAndQuit("TTF_Init");
    return 1;
  }

  auto window {
      SDL_CreateWindow("Handmade", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_RESIZABLE)};
  if (window == nullptr) {
    outputSDLErrorAndQuit("SDL_CreateWindow");
    return 1;
  }

  auto renderer {SDL_CreateRenderer(window, -1, 0)};
  if (renderer == nullptr) {
    outputSDLErrorAndQuit("SDL_CreateRenderer");
    return 1;
  }

  std::stringstream time_text {};
  TTF_Font *Sans {TTF_OpenFont("fonts/OpenSans-Regular.ttf", 11)};
  if (Sans == nullptr) {
    outputTTFErrorAndQuit("TTF_OpenFont");
    return 1;
  }

  SDL_Color White {255, 255, 255, 255};

  SDL_Rect fps_rect;
  fps_rect.x = 0;
  fps_rect.y = 0;

  // Start counting frames per second
  long counted_frames = 0;
  auto fps_timer {std::chrono::steady_clock::now()};
  SDL_Texture *message_texture {};
  while (running) {
    SDL_Event event {};
    while (SDL_PollEvent(&event)) {
      handleEvent(&event);
    }

    int success {SDL_RenderClear(renderer)};
    if (success < 0) {
      outputSDLErrorAndQuit("SDL_RenderClear");
      return 1;
    }

    if (message_texture != nullptr) {
      SDL_DestroyTexture(message_texture);
    }
    message_texture = getFPSAsTexture(fps_timer, counted_frames, &time_text, renderer, Sans, &White, &fps_rect);
    if (message_texture != nullptr) {
      SDL_RenderCopy(renderer, message_texture, NULL, &fps_rect);
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderPresent(renderer);

    ++counted_frames;
  }

  return 0;
}

bool handleEvent(SDL_Event *event) {
  bool should_quit {false};

  switch (event->type) {
  case SDL_QUIT: {
    std::cout << "SDL_QUIT\n";
    running = false;
  } break;

  case SDL_WINDOWEVENT: {
    switch (event->window.event) {
    case SDL_WINDOWEVENT_RESIZED: {
      std::cout << "SDL_WINDOWEVENT_RESIZED ( " << event->window.data1 << "," << event->window.data2 << ")\n";
    } break;

    case SDL_WINDOWEVENT_EXPOSED: {
    } break;
    }
  } break;
  }

  return should_quit;
}

void outputSDLErrorAndQuit(std::string caller) {
  const char *err {SDL_GetError()};
  std::cerr << "SDL error in " << caller << ": " << err << '\n';
}

void outputTTFErrorAndQuit(std::string caller) {
  const char *err {TTF_GetError()};
  std::cerr << "TTF error in " << caller << ": " << err << '\n';
}

SDL_Texture *getFPSAsTexture(std::chrono::steady_clock::time_point start, long counted_frames,
                             std::stringstream *time_text, SDL_Renderer *renderer, TTF_Font *font,
                             SDL_Color *text_color, SDL_Rect *message_rect) {
  float fs = std::chrono::duration<float> {std::chrono::steady_clock::now() - start}.count();
  float avg_fps {static_cast<float>(counted_frames) / fs};

  time_text->str("");
  *time_text << "FPS: " << avg_fps;

  SDL_Surface *message_surface {TTF_RenderText_Solid(font, time_text->str().c_str(), *text_color)};
  if (message_surface == nullptr) {
    outputTTFErrorAndQuit("TTF_RenderText_Solid");
    return nullptr;
  }

  message_rect->w = message_surface->w;
  message_rect->h = message_surface->h;

  auto result = SDL_CreateTextureFromSurface(renderer, message_surface);
  if (result == nullptr) {
    outputSDLErrorAndQuit("SDL_CreateTextureFromSurface");
    return nullptr;
  }
  SDL_FreeSurface(message_surface);
  return result;
}
