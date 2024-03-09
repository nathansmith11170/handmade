module;
#include "SDL.h"

module Platform;

import Handmade;

DebugReadFileResult debugPlatformReadEntireFile(const char *fileName) {
    SDL_RWops *rwOps = SDL_RWFromFile(fileName, "r");
    if (rwOps == nullptr) {
        // TODO(Nathan) logging
        return DebugReadFileResult{nullptr, 0};
    }
    void *data = SDL_LoadFile_RW(rwOps, 0, 0);
    if (data == nullptr) {
        // TODO(Nathan) logging
        return DebugReadFileResult{nullptr, 0};
    }
    int fileSize = rwOps->size(rwOps);
    rwOps->close(rwOps);
    return DebugReadFileResult{data, fileSize};
}

void debugPlatformFreeFileMemory(void *memory) { SDL_free(memory); }

bool debugPlatformWriteEntireFile(const char *fileName, int fileSize,
                                  void *memory) {
    SDL_RWops *rwOps = SDL_RWFromFile(fileName, "w");
    if (rwOps == nullptr) {
        // TODO(Nathan) logging
        return false;
    }
    int written = rwOps->write(rwOps, memory, sizeof(char), fileSize);
    rwOps->close(rwOps);
    return written == fileSize;
}
