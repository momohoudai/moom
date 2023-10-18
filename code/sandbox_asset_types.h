
#ifndef LIT_ASSET_TYPES_H
#define LIT_ASSET_TYPES_H

#include "momo.h"

#define GAME_ASSET_ID_DEFINED

enum game_asset_bitmap_id_t : u32_t {
  ASSET_BITMAP_ID_MAX
};

enum game_asset_sprite_id_t : u32_t {
  ASSET_SPRITE_ID_MAX
};

enum game_asset_font_id_t : u32_t {
  ASSET_FONT_ID_MAX
};

enum game_asset_sound_id_t : u32_t {
  ASSET_SOUND_ID_TEST,
  ASSET_SOUND_ID_MAX
};

#define SANDBOX_ASSET_FILE "sandbox.dat"

#endif
