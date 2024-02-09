#include "handmade.h"

void game_update_and_render(GameOffscreenBuffer *buffer, int blue_offset, int green_offset) {
  u8 *row_ptr = (u8 *)(buffer->memory);
  for (int y = 0; y < buffer->height; ++y) {
    u32 *pixel_ptr = (u32 *)(row_ptr);
    for (int x = 0; x < buffer->width; ++x) {
      u8 blue = (u8)(x + blue_offset);
      u8 green = (u8)(y + green_offset);

      *pixel_ptr = ((u32)(green) << 8) | (u32)(blue);
      pixel_ptr++;
    }
    row_ptr += buffer->pitch;
  }
}
