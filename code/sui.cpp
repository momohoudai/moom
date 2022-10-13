// This is the asset builder tool
//
#include "sui.h"
#include "sui_atlas.h"
#include "sui_pack.h"
#include "karu.h"

#define sui_check_death(e) if (!e) goto fail;

int main() {
  Block block = sui_malloc(MB(100));
  defer { sui_free(&block); };
  make(Bump_Allocator, allocator);
  ba_init(allocator, block.data, block.size);
 
  //make(WAV, loaded_wav);
  //sui_read_wav_from_file(loaded_wav,asset_dir("bgm_menu.wav"), allocator); 
 
#if 0
  sui_log("Building atlas...\n");
  make(Sui_Atlas, atlas);

  if (!begin_atlas_builder(atlas, "BITMAP_DEFAULT", 2048, 2048)) return 1;
  {
    push_sprite(atlas, "SPRITE_BLANK", asset_dir("blank.png"));
    push_sprite(atlas, "SPRITE_BULLET_CIRCLE", asset_dir("bullet_circle.png"));
    push_sprite(atlas, "SPRITE_BULLET_DOT", asset_dir("bullet_dot.png"));
    push_sprite(atlas, "SPRITE_PLAYER_BLACK", asset_dir("player_black.png"));
    push_sprite(atlas, "SPRITE_PLAYER_WHITE", asset_dir("player_white.png"));
    
    U32 interested_cps[] = { 
      32,33,32,34,35,36,37,38,39,
      40,41,42,43,44,45,46,47,48,49,
      50,51,52,53,54,55,56,57,58,59,
      60,61,62,63,64,65,66,67,68,69,
      70,71,72,73,74,75,76,77,78,79,
      80,81,82,83,84,85,86,87,88,89,
      80,81,82,83,84,85,86,87,88,89,
      90,91,92,93,94,95,96,97,98,99,
      100,101,102,103,104,105,106,107,108,109,
      110,111,112,113,114,115,116,117,118,119,
      120,121,122,123,124,125,126,
    };
    
    push_font(atlas, "FONT_DEFAULT", asset_dir("nokiafc22.ttf"), interested_cps, array_count(interested_cps), 128.f);
    
    push_font(atlas, "FONT_DEBUG", asset_dir("liberation-mono.ttf"), interested_cps, array_count(interested_cps), 128.f);
  }
  if(!end_atlas_builder(atlas, allocator)) return 1;
  sui_log("Finished atlas...\n");
#endif  
  
  
  make(Sui_Packer, sp);
  

  sui_check_death(begin_packer(sp, allocator,
                  code_dir("generated_pack_ids.h"),
                  code_dir("generated_bitmap_ids.h"),
                  code_dir("generated_sprite_ids.h"),
                  code_dir("generated_font_ids.h"),
                  code_dir("generated_sound_ids.h")));
  begin_asset_pack(sp);
  // Maybe we want to do something like this:
  begin_atlas(sp);

  begin_atlas_font(sp, "FONT_DEFAULT", asset_dir("nokiafc22.ttf"), 72.f);
  for (U32 i = 32 ; i <= 126; ++i) 
    push_atlas_font_glyph(sp, i);
  end_atlas_font(sp);

  begin_atlas_font(sp, "FONT_DEBUG", asset_dir("liberation-mono.ttf"), 72.f);
  for (U32 i = 32 ; i <= 126; ++i) 
    push_atlas_font_glyph(sp, i);
  end_atlas_font(sp);

  push_atlas_sprite(sp, "SPRITE_BLANK",         asset_dir("blank.png"));
  push_atlas_sprite(sp, "SPRITE_BULLET_CIRCLE", asset_dir("bullet_circle.png"));
  push_atlas_sprite(sp, "SPRITE_BULLET_DOT",    asset_dir("bullet_dot.png"));
  push_atlas_sprite(sp, "SPRITE_PLAYER_BLACK",  asset_dir("player_black.png"));
  push_atlas_sprite(sp, "SPRITE_PLAYER_WHITE",  asset_dir("player_white.png"));
  push_atlas_sprite(sp, "SPRITE_CIRCLE",        asset_dir("circle.png"));
  sui_check_death(end_atlas(sp, "BITMAP_DEFAULT", 1024, 1024));

  //push_sound(sp, "SOUND_TEST", loaded_wav);
  sui_check_death(end_asset_pack(sp, "PACK_DEFAULT", "test.sui"));
  end_packer(sp);

  return 0;

fail:
  sui_log("Something went wrong");
  return 1;

}
