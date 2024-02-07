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

#include "SDL_audio.h"
#include "SDL_video.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

bool running;
u64 countedFrames = 0;
u64 lastTime;
const int BYTES_PER_PIXEL = 4;

u64 fps = 0;
TTF_Font *Sans;
void *fpsText = NULL;
SDL_Color White = {255, 255, 255, 255};
struct SDL_Texture *FpsMessageTexture;

typedef struct SdlOffscreenBuffer {
  // 32bit pixels in BGRX order
  struct SDL_Texture *Texture;
  void *Memory;
  int Width;
  int Height;
  int Pitch;
} SdlOffscreenBuffer;

typedef struct SdlWindowDimension {
  int Width;
  int Height;
} SdlWindowDimension;

typedef struct SdlAudioRingBuffer {
  int Size;
  int WriteCursor;
  int PlayCursor;
  void *Data;
} SdlAudioRingBuffer;

SdlAudioRingBuffer AudioRingBuffer = {};
SdlOffscreenBuffer GlobalBackbuffer = {};
SDL_AudioSpec AudioSettings = {};

void output_sdl_error(const char *caller) {
  const char *err = SDL_GetError();
  printf("SDL error in %s: %s\n", caller, err);
  running = false;
}

void output_ttf_error(const char *caller) {
  const char *err = TTF_GetError();
  printf("TTF error in %s: %s\n", caller, err);
}

SdlWindowDimension sdl_get_window_dimension(SDL_Window *window) {
  SdlWindowDimension result = {};

  SDL_GetWindowSize(window, &result.Width, &result.Height);

  return result;
}

void sdl_audio_callback(void *userData, u8 *audioData, int length) {
  SdlAudioRingBuffer *ringBuffer = (SdlAudioRingBuffer *)userData;

  int region1Size = length;
  int region2Size = 0;
  if (ringBuffer->PlayCursor + length > ringBuffer->Size) {
    region1Size = ringBuffer->Size - ringBuffer->PlayCursor;
    region2Size = length - region1Size;
  }

  memcpy(audioData, (u8 *)(ringBuffer->Data) + ringBuffer->PlayCursor, (size_t)(region1Size));
  memcpy(&audioData[region1Size], ringBuffer->Data, (size_t)(region2Size));

  ringBuffer->PlayCursor = (ringBuffer->PlayCursor + length) % ringBuffer->Size;
  ringBuffer->WriteCursor = (ringBuffer->PlayCursor + 2048) % ringBuffer->Size;
}

void sdl_init_sound_device(i32 samplesPerSecond, i32 bufferSize) {
  AudioSettings.freq = samplesPerSecond;
  AudioSettings.format = AUDIO_S16LSB;
  AudioSettings.channels = 2;
  AudioSettings.samples = 1024;
  AudioSettings.callback = &sdl_audio_callback;
  AudioSettings.userdata = &AudioRingBuffer;

  AudioRingBuffer.Size = bufferSize;
  AudioRingBuffer.Data = malloc((u32)(bufferSize));
  AudioRingBuffer.PlayCursor = AudioRingBuffer.WriteCursor = 0;

  SDL_OpenAudio(&AudioSettings, nullptr);

  printf("Initialised an Audio device at frequency %d Hz, %d Channels, buffer "
         "size %d\n",
         AudioSettings.freq, AudioSettings.channels, AudioSettings.samples);

  if (AudioSettings.format != AUDIO_S16LSB) {
    printf("Oops! We didn't get AUDIO_S16LSB as our sample format!\n");
    SDL_CloseAudio();
  }
}

SDL_Texture *get_fps_texture(SDL_Renderer *renderer, SDL_Rect *messageRect) {
  if (FpsMessageTexture != nullptr) {
    SDL_DestroyTexture(FpsMessageTexture);
  }
  if (fpsText != nullptr) {
    free(fpsText);
  }

  size_t size = (size_t)snprintf(NULL, 0, "FPS: %lu", fps);
  fpsText = (char *)malloc(size + 1);
  snprintf((char *)(fpsText), size + 1, "FPS: %lu", fps);

  SDL_Surface *messageSurface = TTF_RenderText_Solid(Sans, fpsText, White);
  if (messageSurface == nullptr) {
    output_ttf_error("TTF_RenderText_Solid");
    return NULL;
  }

  messageRect->w = messageSurface->w;
  messageRect->h = messageSurface->h;

  SDL_Texture *result = SDL_CreateTextureFromSurface(renderer, messageSurface);
  if (result == nullptr) {
    output_sdl_error("SDL_CreateTextureFromSurface");
    return NULL;
  }
  SDL_FreeSurface(messageSurface);
  return result;
}

void sdl_resize_texture(SDL_Renderer *renderer, int width, int height) {
  if (GlobalBackbuffer.Texture) {
    SDL_DestroyTexture(GlobalBackbuffer.Texture);
  }
  if (GlobalBackbuffer.Memory) {
    munmap(GlobalBackbuffer.Memory, (size_t)(GlobalBackbuffer.Width * GlobalBackbuffer.Height * BYTES_PER_PIXEL));
  }
  GlobalBackbuffer.Texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
  GlobalBackbuffer.Width = width;
  GlobalBackbuffer.Height = height;
  GlobalBackbuffer.Pitch = width * BYTES_PER_PIXEL;
  GlobalBackbuffer.Memory = mmap(0, (size_t)(GlobalBackbuffer.Width * GlobalBackbuffer.Height * BYTES_PER_PIXEL),
                                 PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

void sdl_update_window(SDL_Renderer *renderer) {
  u8 *textureData = NULL;
  int texturePitch = 0;

  SDL_LockTexture(GlobalBackbuffer.Texture, nullptr, (void **)&textureData, &texturePitch);
  memcpy(textureData, GlobalBackbuffer.Memory, (size_t)(GlobalBackbuffer.Height * GlobalBackbuffer.Pitch) * sizeof(u8));
  SDL_UnlockTexture(GlobalBackbuffer.Texture);

  SDL_RenderCopy(renderer, GlobalBackbuffer.Texture, NULL, NULL);

  SDL_Rect fpsRect;
  fpsRect.x = 0;
  fpsRect.y = 0;
  FpsMessageTexture = get_fps_texture(renderer, &fpsRect);
  if (FpsMessageTexture != nullptr) {
    SDL_RenderCopy(renderer, FpsMessageTexture, NULL, &fpsRect);
  }

  SDL_RenderPresent(renderer);
}

void renderWeirdGradient(int xOffset, int yOffset) {
  u8 *row = (u8 *)GlobalBackbuffer.Memory;
  for (int y = 0; y < GlobalBackbuffer.Height; ++y) {
    u32 *pixel = (u32 *)row;
    for (int x = 0; x < GlobalBackbuffer.Width; ++x) {
      u8 blue = (u8)(x + xOffset);
      u8 green = (u8)(y + yOffset);

      *pixel++ = (u32)((green << 8) | blue);
    }

    row += GlobalBackbuffer.Pitch;
  }
}

void handle_event(SDL_Event *event, int *yOffset) {
  switch (event->type) {
  case SDL_QUIT: {
    printf("SDL_QUIT\n");
    running = false;
  } break;

  case SDL_KEYDOWN:
  case SDL_KEYUP: {
    SDL_Keycode keyCode = event->key.keysym.sym;
    // bool was_down = false;
    // if (event->key.state == SDL_RELEASED) {
    //   was_down = true;
    // } else if (event->key.repeat != 0) {
    //   was_down = true;
    // }

    if (keyCode == SDLK_w && event->key.state == SDL_PRESSED) {
      *yOffset += 2;
    }
  } break;

  case SDL_WINDOWEVENT: {
    switch (event->window.event) {
    case SDL_WINDOWEVENT_SIZE_CHANGED: {
      printf("SDL_WINDOWEVENT_RESIZED (%d, %d)\n", event->window.data1, event->window.data2);
    } break;

    case SDL_WINDOWEVENT_FOCUS_GAINED: {
      printf("SDL_WINDOWEVENT_FOCUS_GAINED\n");
    } break;

    case SDL_WINDOWEVENT_EXPOSED: {
      SDL_Window *window = SDL_GetWindowFromID(event->window.windowID);
      SDL_Renderer *renderer = SDL_GetRenderer(window);
      sdl_update_window(renderer);
    } break;
    }
  } break;
  }
}

int main() {
  int resultCode = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  if (resultCode < 0) {
    output_sdl_error("SDL_Init");
    return 1;
  }

  resultCode = TTF_Init();
  if (resultCode < 0) {
    output_ttf_error("TTF_Init");
  }

  struct SDL_Window *window =
      SDL_CreateWindow("Handmade", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1028, 720, SDL_WINDOW_RESIZABLE);
  if (window == nullptr) {
    output_sdl_error("SDL_CreateWindow");
    return 1;
  }

  struct SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
  if (renderer == nullptr) {
    output_sdl_error("SDL_CreateRenderer");
  }

  running = true;

  Sans = TTF_OpenFont("fonts/OpenSans-Regular.ttf", 11);
  if (Sans == nullptr) {
    output_ttf_error("TTF_OpenFont");
    return 1;
  }

  GlobalBackbuffer.Texture = nullptr;
  GlobalBackbuffer.Memory = nullptr;
  GlobalBackbuffer.Height = 0;
  GlobalBackbuffer.Width = 0;
  GlobalBackbuffer.Pitch = 0;

  // Start counting frames per second
  countedFrames = 0;
  lastTime = SDL_GetTicks64();

  int xOffset = 0;
  int yOffset = 0;
  SdlWindowDimension window_dimensions = sdl_get_window_dimension(window);
  sdl_resize_texture(renderer, window_dimensions.Width, window_dimensions.Height);

  // NOTE: Sound test
  int samplesPerSecond = 48000;
  int toneHz = 256;
  i16 toneVolume = 3000;
  u32 runningSampleIndex = 0;
  int squareWavePeriod = samplesPerSecond / toneHz;
  int halfSquareWavePeriod = squareWavePeriod / 2;
  int bytesPerSample = sizeof(i16) * 2;
  int secondaryBufferSize = samplesPerSecond * bytesPerSample;

  sdl_init_sound_device(samplesPerSecond, secondaryBufferSize);
  bool isSoundPlaying = false;

  while (running) {
    u64 currentTime = SDL_GetTicks64();
    if (currentTime - lastTime >= 1000) {
      // Calculate frames per second
      fps = countedFrames;
      countedFrames = 0;
      lastTime = currentTime;
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      handle_event(&event, &yOffset);
    }
    renderWeirdGradient(xOffset, yOffset);

    // Sound square wave test
    SDL_LockAudio();
    int byteToLock = (int)(runningSampleIndex)*bytesPerSample % secondaryBufferSize;
    int bytesToWrite;
    if (byteToLock == AudioRingBuffer.PlayCursor) {
      bytesToWrite = secondaryBufferSize;
    } else if (byteToLock > AudioRingBuffer.PlayCursor) {
      bytesToWrite = (secondaryBufferSize - byteToLock);
      bytesToWrite += AudioRingBuffer.PlayCursor;
    } else {
      bytesToWrite = AudioRingBuffer.PlayCursor - byteToLock;
    }

    void *region1 = (u8 *)AudioRingBuffer.Data + byteToLock;
    int region1Size = bytesToWrite;
    if (region1Size + byteToLock > secondaryBufferSize) {
      region1Size = secondaryBufferSize - byteToLock;
    }
    void *region2 = AudioRingBuffer.Data;
    int region2Size = bytesToWrite - region1Size;
    SDL_UnlockAudio();
    int region1SampleCount = region1Size / bytesPerSample;
    i16 *sampleOut = (i16 *)region1;
    for (int sampleIndex = 0; sampleIndex < region1SampleCount; ++sampleIndex) {
      i16 sampleValue = (((int)(runningSampleIndex++) / halfSquareWavePeriod) % 2) ? toneVolume : -toneVolume;

      *sampleOut++ = sampleValue;
      *sampleOut++ = sampleValue;
    }

    int region2SampleCount = region2Size / bytesPerSample;
    sampleOut = (i16 *)region2;
    for (int sampleIndex = 0; sampleIndex < region2SampleCount; ++sampleIndex) {
      i16 sampleValue = (((int)(runningSampleIndex++) / halfSquareWavePeriod) % 2) ? toneVolume : toneVolume;

      *sampleOut++ = sampleValue;
      *sampleOut++ = sampleValue;
    }

    if (!isSoundPlaying) {
      SDL_PauseAudio(0);
      isSoundPlaying = true;
    }

    sdl_update_window(renderer);

    ++xOffset;
    ++countedFrames;
  }

  SDL_Quit();
  return 0;
}
