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

#include "fixed_time.cpp"
#include "handmade.cpp"
#include "handmade.h"
#include <SDL2/SDL.h>
#include <SDL_audio.h>
#include <SDL_events.h>
#include <SDL_keyboard.h>
#include <SDL_keycode.h>
#include <SDL_pixels.h>
#include <SDL_render.h>
#include <SDL_rwops.h>
#include <SDL_timer.h>
#include <SDL_video.h>
#include <iostream>

struct SdlContext {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *screen;
  SDL_AudioSpec audioDevice;
  SDL_AudioDeviceID audioDeviceId;
};

static void drawBegin(SdlContext sdlc) { SDL_RenderClear(sdlc.renderer); }
static void drawFrame(SdlContext sdlc, GameOffscreenBuffer offscreenBuf) {
  int32_t screenWidth{};
  int32_t screenHeight{};
  int32_t screenPitch{};
  void *screenPixels{};
  SDL_QueryTexture(sdlc.screen, nullptr, nullptr, &screenWidth, &screenHeight);
  SDL_LockTexture(sdlc.screen, nullptr, &screenPixels, &screenPitch);
  memcpy(screenPixels, offscreenBuf.memory, screenHeight * screenPitch);
  SDL_UnlockTexture(sdlc.screen);
  SDL_RenderCopy(sdlc.renderer, sdlc.screen, nullptr, nullptr);
}
static void drawEnd(SdlContext sdlc) { SDL_RenderPresent(sdlc.renderer); }

// static DebugReadFileResult debugPlatformReadEntireFile(const char *fileName)
// {
//   SDL_RWops *rwOps = SDL_RWFromFile(fileName, "r");
//   if (rwOps == nullptr) {
//     // TODO(Nathan) logging
//     return DebugReadFileResult{nullptr, 0};
//   }
//   void *data = SDL_LoadFile_RW(rwOps, 0, 0);
//   if (data == nullptr) {
//     // TODO(Nathan) logging
//     return DebugReadFileResult{nullptr, 0};
//   }
//   int fileSize = rwOps->size(rwOps);
//   rwOps->close(rwOps);
//   return DebugReadFileResult{data, fileSize};
// }

// static void debugPlatformFreeFileMemory(void *memory) { SDL_free(memory); }

// static bool debugPlatformWriteEntireFile(const char *fileName, int fileSize,
//                                          void *memory) {
//   SDL_RWops *rwOps = SDL_RWFromFile(fileName, "w");
//   if (rwOps == nullptr) {
//     // TODO(Nathan) logging
//     return false;
//   }
//   int written = rwOps->write(rwOps, memory, sizeof(char), fileSize);
//   rwOps->close(rwOps);
//   return written == fileSize;
// }

int main() {
  SdlContext sdlc{};
  auto result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  if (result < 0) {
    const char *err = SDL_GetError();
    std::cout << "Error initializing SDL: " << err << '\n';
  }

  sdlc.window =
      SDL_CreateWindow("Handmade", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, 1024, 768, SDL_WINDOW_SHOWN);
  if (sdlc.window == nullptr) {
    const char *err = SDL_GetError();
    std::cout << "Error creating window: " << err << '\n';
  }

  sdlc.renderer = SDL_CreateRenderer(sdlc.window, 0, SDL_RENDERER_ACCELERATED);
  if (sdlc.renderer == nullptr) {
    const char *err = SDL_GetError();
    std::cout << "Error creating renderer: " << err << '\n';
  }

  sdlc.screen = SDL_CreateTexture(sdlc.renderer, SDL_PIXELFORMAT_ARGB8888,
                                  SDL_TEXTUREACCESS_STREAMING, 1024, 768);
  if (sdlc.screen == nullptr) {
    const char *err = SDL_GetError();
    std::cout << "Error creating texture: " << err << '\n';
  }

  auto requestedSpec =
      SDL_AudioSpec{44100, AUDIO_S16LSB, 2, 0, 0, 0, 0, nullptr, nullptr};
  sdlc.audioDeviceId = SDL_OpenAudioDevice(
      nullptr, false, &requestedSpec, nullptr,
      SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
  sdlc.audioDevice = requestedSpec;
  if (sdlc.audioDeviceId == 0) {
    const char *err = SDL_GetError();
    std::cout << "Error opening audio device: " << err << '\n';
  }

  GameMemory gameMemory{};
  gameMemory.permanentStorageSize = 64ll * 1024ll * 1024ll;
  gameMemory.permanentStorage = (unsigned char *)malloc(64ll * 1024ll * 1024ll);
  gameMemory.transientStorageSize = 4ll * 1024ll * 1024ll * 1024ll;
  gameMemory.transientStorage =
      (unsigned char *)malloc(4ll * 1024ll * 1024ll * 1024ll);

  GameOffscreenBuffer gameOffscreenBuffer{};
  gameOffscreenBuffer.width = 1024;
  gameOffscreenBuffer.height = 768;
  gameOffscreenBuffer.pitch = 1024 * 4;
  gameOffscreenBuffer.memory = (unsigned char *)malloc(
      gameOffscreenBuffer.pitch * gameOffscreenBuffer.height);

  GameSoundBuffer soundBuffer{};
  soundBuffer.samplesPerSec = sdlc.audioDevice.freq;
  soundBuffer.bytesPerSample = SDL_AUDIO_BITSIZE(sdlc.audioDevice.format) / 8 *
                               sdlc.audioDevice.channels;
  soundBuffer.enabled = sdlc.audioDeviceId != 0;
  if (soundBuffer.enabled) {
    soundBuffer.memory =
        (short *)malloc(sdlc.audioDevice.freq * soundBuffer.bytesPerSample *
                        sdlc.audioDevice.channels);
  }

  GameInput gameInput{};

  bool shouldClose{false};
  float gameTickSeconds{0.0333333333f};
  Time64 gameClock{};
  float accumulator{};
  float fpsPrintDelay{};
  int64_t frameCount{};

  uint64_t lastCounter = SDL_GetPerformanceCounter();
  bool isAudioPaused{true};
  updateGame(&gameMemory, &gameInput);
  while (!shouldClose) {
    SDL_Event evt{};
    while (SDL_PollEvent(&evt)) {
      switch (evt.type) {
      case SDL_QUIT: {
        shouldClose = true;
      } break;
      case SDL_KEYDOWN: {
        if (evt.key.keysym.sym == SDLK_w) {
          gameInput.speedUp.halfTransitionCount += 1;
          gameInput.speedUp.endedDown = true;
        }
        if (evt.key.keysym.sym == SDLK_a) {
          gameInput.strafeLeft.halfTransitionCount += 1;
          gameInput.strafeLeft.endedDown = true;
        }
        if (evt.key.keysym.sym == SDLK_s) {
          gameInput.speedDown.halfTransitionCount += 1;
          gameInput.speedDown.endedDown = true;
        }
        if (evt.key.keysym.sym == SDLK_d) {
          gameInput.strafeRight.halfTransitionCount += 1;
          gameInput.strafeRight.endedDown = true;
        }
        if (evt.key.keysym.sym == SDLK_ESCAPE) {
          shouldClose = true;
        }
      } break;
      case SDL_KEYUP: {
        if (evt.key.keysym.sym == SDLK_w) {
          gameInput.speedUp.halfTransitionCount += 1;
          gameInput.speedUp.endedDown = false;
        }
        if (evt.key.keysym.sym == SDLK_a) {
          gameInput.strafeLeft.halfTransitionCount += 1;
          gameInput.strafeLeft.endedDown = false;
        }
        if (evt.key.keysym.sym == SDLK_s) {
          gameInput.speedDown.halfTransitionCount += 1;
          gameInput.speedDown.endedDown = false;
        }
        if (evt.key.keysym.sym == SDLK_d) {
          gameInput.strafeRight.halfTransitionCount += 1;
          gameInput.strafeRight.endedDown = false;
        }
      } break;
      case SDL_WINDOWEVENT: {
        // TODO(Nathan) window events
      } break;
      }
    }

    uint64_t currentCounter = SDL_GetPerformanceCounter();
    float frameTime = (float)(currentCounter - lastCounter) /
                      (float)(SDL_GetPerformanceFrequency());
    lastCounter = currentCounter;
    accumulator += frameTime;
    fpsPrintDelay += frameTime;
    frameCount++;
    if (fpsPrintDelay > 1.0f) {
      std::cout << "Estimated FPS: " << frameCount << '\n';
      fpsPrintDelay = 0.0f;
      frameCount = 0;
    }

    while (accumulator > gameTickSeconds) {
      updateGame(&gameMemory, &gameInput);
      gameClock = Time64AddFloat(gameClock, gameTickSeconds);
      accumulator -= gameTickSeconds;
    }

    int32_t targetQueueBytes =
        (int32_t)((float)(soundBuffer.samplesPerSec *
                          soundBuffer.bytesPerSample * gameTickSeconds * 2) +
                  0.5f);
    int32_t bytesToWrite =
        targetQueueBytes - SDL_GetQueuedAudioSize(sdlc.audioDeviceId);
    soundBuffer.samplesNeeded =
        bytesToWrite > 0 ? bytesToWrite / soundBuffer.bytesPerSample : 0;
    fillBuffers(&gameMemory, &gameOffscreenBuffer, &soundBuffer);
    if (soundBuffer.enabled) {
      SDL_QueueAudio(sdlc.audioDeviceId, soundBuffer.memory, bytesToWrite);
    }

    drawBegin(sdlc);
    drawFrame(sdlc, gameOffscreenBuffer);
    drawEnd(sdlc);

    if (isAudioPaused && soundBuffer.enabled) {
      isAudioPaused = false;
      SDL_PauseAudioDevice(sdlc.audioDeviceId, false);
    }
  }

  if (sdlc.audioDeviceId != 0) {
    SDL_CloseAudioDevice(sdlc.audioDeviceId);
  }
  SDL_DestroyTexture(sdlc.screen);
  SDL_DestroyRenderer(sdlc.renderer);
  SDL_DestroyWindow(sdlc.window);
  SDL_Quit();
}
