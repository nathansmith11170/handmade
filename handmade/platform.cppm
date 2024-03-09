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
export module Platform;

import Handmade;

export DebugReadFileResult debugPlatformReadEntireFile(const char *fileName);
export void debugPlatformFreeFileMemory(void *memory);
export bool debugPlatformWriteEntireFile(const char *fileName, int fileSize,
                                         void *memory);
