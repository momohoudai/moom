#ifndef GAME_ASSET_IDS
#define GAME_ASSET_IDS

#include "momo.h"

// @note: for now this is only GLSL
enum eden_asset_shader_id_t : u32_t {
  ASSET_SHADER_ID_VERTEX,
  ASSET_SHADER_ID_PIXEL,

  ASSET_SHADER_ID_MAX
};


enum eden_asset_bitmap_id_t : u32_t {
  ASSET_BITMAP_ID_ATLAS,

  ASSET_BITMAP_ID_MAX
};

enum eden_asset_sprite_id_t : u32_t {
  ASSET_SPRITE_ID_BLANK_SPRITE,
  ASSET_SPRITE_ID_CIRCLE_SPRITE,
  ASSET_SPRITE_ID_FILLED_CIRCLE_SPRITE,
  ASSET_SPRITE_ID_MOVE_SPRITE,
  ASSET_SPRITE_ID_ROTATE_SPRITE,

  ASSET_SPRITE_ID_MAX
};

enum eden_asset_font_id_t : u32_t {
  ASSET_FONT_ID_DEFAULT,
  ASSET_FONT_ID_DEBUG,

  ASSET_FONT_ID_MAX
};

enum eden_asset_sound_id_t : u32_t {
  ASSET_SOUND_BGM0,
  ASSET_SOUND_BGM1,
  ASSET_SOUND_BGM2,
  ASSET_SOUND_BGM3,

  ASSET_SOUND_PICKUP,
  ASSET_SOUND_PUTDOWN,
  ASSET_SOUND_DONE,

  ASSET_SOUND_ID_MAX
};

#define LIT_ASSET_FILE "lit.dat"

#endif
