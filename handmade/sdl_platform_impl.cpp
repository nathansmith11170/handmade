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
module;
#include "SDL.h"
#include <cstring>
#include <string>
module Platform;

DebugReadFileResult debugPlatformReadEntireFile(std::string fileName) {
    SDL_RWops *rwOps = SDL_RWFromFile(fileName.c_str(), "r");
    if (rwOps == nullptr) {
        // TODO(Nathan) logging
        return DebugReadFileResult{nullptr, 0};
    }
    void *data = SDL_LoadFile_RW(rwOps, nullptr, 0);
    if (data == nullptr) {
        // TODO(Nathan) logging
        rwOps->close(rwOps);
        return DebugReadFileResult{nullptr, 0};
    }
    int fileSize = rwOps->size(rwOps);
    rwOps->close(rwOps);
    return DebugReadFileResult{data, fileSize};
}

void debugPlatformFreeFileMemory(void *memory) { SDL_free(memory); }

bool debugPlatformWriteEntireFile(std::string fileName, int fileSize,
                                  void *memory) {
    SDL_RWops *rwOps = SDL_RWFromFile(fileName.c_str(), "w");
    if (rwOps == nullptr) {
        // TODO(Nathan) logging
        return false;
    }
    int written = rwOps->write(rwOps, memory, sizeof(char), fileSize);
    rwOps->close(rwOps);
    return written == fileSize;
}
