export module Platform;

import Handmade;

export DebugReadFileResult debugPlatformReadEntireFile(const char *fileName);
export void debugPlatformFreeFileMemory(void *memory);
export bool debugPlatformWriteEntireFile(const char *fileName, int fileSize,
                                         void *memory);
