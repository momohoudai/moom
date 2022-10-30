#include <stdlib.h>
#include <stdio.h>

#include "sui.h"





int main() {
  Block block = sui_malloc(MB(100));
  defer { sui_free(&block); };
  make(Bump_Allocator, allocator);
  ba_init(allocator, block.data, block.size);

  sui_log("Building atlas...\n");
  make(Sui_Atlas, atlas);

  sui_atlas_begin(atlas, 1024, 1024);
  Sui_Atlas_Sprite* blank_sprite = sui_atlas_push_sprite(atlas, sui_asset_dir("blank.png"));
  Sui_Atlas_Sprite* circle_sprite = sui_atlas_push_sprite(atlas, sui_asset_dir("circle.png"));
    
  sui_atlas_begin_font(atlas, sui_asset_dir("nokiafc22.ttf"), 72.f);
  for (U32 i = 32; i <= 126; ++i){
    sui_atlas_push_font_codepoint(atlas, i);
  }
  auto* font_a = sui_atlas_end_font(atlas);

  sui_atlas_begin_font(atlas, sui_asset_dir("liberation-mono.ttf"), 72.f);
  for (U32 i = 32; i <= 126; ++i){
    sui_atlas_push_font_codepoint(atlas, i);
  }
  auto* font_b = sui_atlas_end_font(atlas);
  sui_atlas_end(atlas, allocator);
  sui_log("Finished atlas...\n");

#if 0
  sui_log("Writing test png file...\n");
  Block png_to_write_memory = png_write_img32_to_blk(atlas->bitmap, allocator);
  sui_write_file_from_blk("test.png", png_to_write_memory);
#endif

  make(Sui_Packer, packer);
  sui_pack_begin(packer);

  sui_pack_begin_group(packer, GAME_ASSET_GROUP_TYPE_ATLAS);
  U32 bitmap_id = sui_pack_push_bitmap(packer, atlas->bitmap.width, atlas->bitmap.height, atlas->bitmap.pixels);
  sui_pack_end_group(packer);
  
  sui_pack_begin_group(packer, GAME_ASSET_GROUP_TYPE_BLANK_SPRITE);
  sui_pack_push_sprite(packer, bitmap_id, sui_rp_rect_to_rect2u(*blank_sprite->rect));
  sui_pack_end_group(packer);

  sui_pack_begin_group(packer, GAME_ASSET_GROUP_TYPE_CIRCLE_SPRITE);
  sui_pack_push_sprite(packer, bitmap_id, sui_rp_rect_to_rect2u(*circle_sprite->rect));
  sui_pack_end_group(packer);

  sui_pack_begin_group(packer, GAME_ASSET_GROUP_TYPE_DEFAULT_FONT);
  sui_pack_push_font(packer, font_a, bitmap_id);
  sui_pack_end_group(packer);

  sui_pack_end(packer, "test_pack.sui", allocator);
}


