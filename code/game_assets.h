#ifndef GAME_ASSETS_H
#define GAME_ASSETS_H

#include "game_asset_types.h"
#include "sui.h"

struct Asset_Bitmap_ID { U32 value; };
struct Asset_Font_ID { U32 value; }; 
struct Asset_Image_ID { U32 value; };

struct Bitmap_Asset {
  U32 gfx_bitmap_id; // TODO: tie in with renderer? 
  
  U32 width;
  U32 height;
  U32* pixels;
};

struct Image_Asset {
  Rect2 uv;
  Asset_Bitmap_ID bitmap_id;
};

struct Font_Glyph_Asset{
  Rect2 uv;
  Asset_Bitmap_ID bitmap_id;
};

struct Font_Asset {
  U32 one_past_highest_codepoint;
  U16* codepoint_map;
  
  U32 glyph_count;
  Font_Glyph_Asset* glyphs;
  F32* horizontal_advances;
};

struct Asset {
  U32 first_tag_index;
  U32 one_past_last_tag_index;
  
  Asset_Type type;
  union {
    Bitmap_Asset bitmap;
    Image_Asset image;
    Font_Asset font;
  };
};

struct Asset_Group {
  U32 first_asset_index;
  U32 one_past_last_asset_index;
};

struct Asset_Vector {
  F32 e[ASSET_TAG_TYPE_COUNT];
};

struct Asset_Tag {
  Asset_Tag_Type type;
  F32 value;
};

struct Game_Assets {
  Arena arena;
  
  U32 asset_count;
  Asset* assets;
  
  U32 tag_count;
  Asset_Tag* tags;
  
  Asset_Group groups[ASSET_GROUP_COUNT];
  
  
  // TODO(Momo): We should remove this
  U32 bitmap_counter;
};

#include "game_assets.cpp"

#endif //GAME_ASSETS_H
