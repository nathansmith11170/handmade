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
#include <iostream>

bool HandleEvent(SDL_Event *Event);

int main(int argc, char *argv[]) {
  int call_code {SDL_InitSubSystem(SDL_INIT_VIDEO)};
  if (call_code != 0) {
    const char *err {SDL_GetError()};
    std::cerr << err << '\n';
    SDL_Quit();
    return 1;
  }

  auto window {
      SDL_CreateWindow("Handmade", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_RESIZABLE)};

  if (!window) {
    const char *err {SDL_GetError()};
    std::cerr << err << '\n';
    SDL_Quit();
    return 1;
  }

  auto renderer {SDL_CreateRenderer(window, -1, 0)};

  while (true) {
    SDL_Event event {};
    SDL_WaitEvent(&event);
    if (HandleEvent(&event)) {
      break;
    }
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
  }

  SDL_Quit();
}

bool HandleEvent(SDL_Event *event) {
  bool should_quit = false;

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
      auto window {SDL_GetWindowFromID(event->window.windowID)};
      auto renderer {SDL_GetRenderer(window)};
      static bool IsWhite = true;
      if (IsWhite == true) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        IsWhite = false;
      } else {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        IsWhite = true;
      }
    } break;
    }
  } break;
  }

  return (should_quit);
}
