#ifndef LIT_ASSET_TYPES_H
#define LIT_ASSET_TYPES_H

#include "momo.h"

#define GAME_ASSET_ID_DEFINED

enum game_asset_bitmap_id_t : u32_t {
  ASSET_BITMAP_ID_ATLAS,

  ASSET_BITMAP_ID_MAX
};

enum game_asset_sprite_id_t : u32_t {
  ASSET_SPRITE_ID_BLANK_SPRITE,
  ASSET_SPRITE_ID_CIRCLE_SPRITE,
  ASSET_SPRITE_ID_FILLED_CIRCLE_SPRITE,
  ASSET_SPRITE_ID_MOVE_SPRITE,
  ASSET_SPRITE_ID_ROTATE_SPRITE,

  ASSET_SPRITE_ID_MAX
};

enum game_asset_font_id_t : u32_t {
  ASSET_FONT_ID_DEFAULT,
  ASSET_FONT_ID_DEBUG,

  ASSET_FONT_ID_MAX
};

enum game_asset_sound_id_t : u32_t {
  ASSET_SOUND_ID_MAX
};

#define LIT_ASSET_FILE "lit.dat"

#endif
