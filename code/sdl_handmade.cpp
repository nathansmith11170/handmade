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

bool handleEvent(SDL_Event *Event);
void outputSDLErrorAndQuit(std::string caller);
void outputTTFErrorAndQuit(std::string caller);

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
  auto fps_timer {std::chrono::high_resolution_clock::now()};
  bool running {true};
  while (running) {
    SDL_Event event {};
    while (SDL_PollEvent(&event)) {
      if (handleEvent(&event)) {
        running = false;
      }
    }

    float fs = std::chrono::duration<float> {std::chrono::high_resolution_clock::now() - fps_timer}.count();
    float avg_fps {counted_frames / fs};

    time_text.str("");
    time_text << "FPS: " << avg_fps;

    SDL_Surface *message_surface {TTF_RenderText_Solid(Sans, time_text.str().c_str(), White)};
    if (message_surface == nullptr) {
      outputTTFErrorAndQuit("TTF_RenderText_Solid");
      return 1;
    }
    fps_rect.w = message_surface->w;
    fps_rect.h = message_surface->h;
    SDL_Texture *message_texture {SDL_CreateTextureFromSurface(renderer, message_surface)};
    if (message_texture == nullptr) {
      outputSDLErrorAndQuit("SDL_CreateTextureFromSurface");
      return 1;
    }

    int success {SDL_RenderClear(renderer)};
    if (success < 0) {
      outputSDLErrorAndQuit("SDL_RenderClear");
      return 1;
    }
    SDL_RenderCopy(renderer, message_texture, NULL, &fps_rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderPresent(renderer);
    SDL_FreeSurface(message_surface);
    SDL_DestroyTexture(message_texture);
    ++counted_frames;
  }

  SDL_Quit();
  TTF_Quit();
}

bool handleEvent(SDL_Event *event) {
  bool should_quit {false};

  switch (event->type) {
  case SDL_QUIT: {
    std::cout << "SDL_QUIT\n";
    should_quit = true;
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
  SDL_Quit();
}

void outputTTFErrorAndQuit(std::string caller) {
  const char *err {TTF_GetError()};
  std::cerr << "TTF error in " << caller << ": " << err << '\n';
  TTF_Quit();
}
