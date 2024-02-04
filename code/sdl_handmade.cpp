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
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", err, 0);
  }

  auto window =
      SDL_CreateWindow("Handmade", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_RESIZABLE);

  while (true) {
    SDL_Event Event {};
    SDL_WaitEvent(&Event);
    if (HandleEvent(&Event)) {
      break;
    }
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
    }
  } break;
  }

  return (should_quit);
}
