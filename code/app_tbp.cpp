
//
// "Tile Based Platformer"
// 
#include "momo.h"
#include "eden_asset_id_lit.h"
#include "eden.h"

#define TBG_TILE_SIZE (50.f)
#define TBG_TILE_SIZE_V2 (v2f_set(TBG_TILE_SIZE, TBG_TILE_SIZE))

#define two_2_one(x, y, w) (x + (w * y))
#define one_2_two(id, w)   (v2u_set(id%w, id/w))

#define tbg_from_eden(eden) ((tbg_t*)eden->user_data)

u32_t g_grid_w = 10;
u32_t g_grid_h = 10;
u32_t g_grid[] = {
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,1,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
};


struct tbg_t {

  arena_t main_arena;
  arena_t asset_arena;
  arena_t debug_arena;
  arena_t frame_arena;

  eden_assets_t assets;
};

// Globals
tbg_t* g_tbg;
eden_t* g_eden;

exported 
eden_update_and_render_sig(eden_update_and_render) 
{ 
  g_eden = eden;
  g_tbg = tbg_from_eden(eden);
  if(eden->user_data == nullptr) {
    usz_t lit_memory_size = sizeof(tbg_t);
    usz_t asset_memory_size = megabytes(20);
    //usz_t debug_memory_size = megabytes(1);
    usz_t frame_memory_size = megabytes(1);
    //usz_t mode_memory_size = megabytes(1);

    // Calculate the total required memory with alignment
    usz_t required_memory = 0;
    {
      make(arena_calc_t, c);
      arena_calc_push(c, lit_memory_size, 16);
      arena_calc_push(c, asset_memory_size, 16);
      //arena_calc_push(c, debug_memory_size, 16);
      arena_calc_push(c, frame_memory_size, 16);
      //arena_calc_push(c, mode_memory_size, 16);
      required_memory = arena_calc_get_result(c);
    }

    u8_t* memory = (u8_t*)eden_allocate_memory(g_eden, required_memory);
    if (!memory) {
      eden->is_running = false;
      return;
    }
    eden->user_data = memory;
    g_tbg = tbg_from_eden(eden);

    //
    // Initialize arenas
    //
    arena_init(&g_tbg->main_arena, memory + sizeof(tbg_t), required_memory - sizeof(tbg_t));
    arena_push_partition(&g_tbg->main_arena, &g_tbg->asset_arena, asset_memory_size, 16);
    arena_push_partition(&g_tbg->main_arena, &g_tbg->frame_arena, asset_memory_size, 16);


    // Initialize assets
    eden_assets_init(&g_tbg->assets, g_eden, LIT_ASSET_FILE, &g_tbg->asset_arena);
    
    // Initialize view
    eden_set_design_dimensions(g_eden, 800, 800);
    eden_set_view(g_eden, 0.f, 800, 0.f, 800, 0.f, 0.f);
  }

  arena_clear(&g_tbg->frame_arena);
  eden_clear_canvas(g_eden, rgba_set(0.2f, 0.2f, 0.2f, 1.f)); 
  eden_set_blend_alpha(g_eden);

  

  // Render grid
  for(u32_t x = 0; x < 10; ++x){
    for (u32_t y = 0; y < 10; ++y) {
      if (g_grid[two_2_one(x,y,10)] == 0) {
        eden_draw_rect(g_eden, v2f_set(x * 50, y * 50), 0, v2f_set(45, 45), rgba_set(0.5f,0.5f,0.5f,1));
      }
    }
  }
  eden_advance_depth(g_eden);

}

//
// Game functions
// 
exported 
eden_get_config_sig(eden_get_config) 
{
  eden_config_t ret;

  ret.target_frame_rate = 60;

  ret.debug_arena_size = kilobytes(300);
  ret.max_inspector_entries = 256;
  ret.max_profiler_entries = 256;
  ret.max_profiler_snapshots = 120;

  ret.gfx_arena_size = megabytes(256);
  ret.texture_queue_size = megabytes(5);
  ret.render_command_size = megabytes(100);
  ret.max_textures = 1;
  ret.max_texture_payloads = 1;
  ret.max_sprites = 4096;
  ret.max_triangles = 4096;

  ret.audio_enabled = false;
  ret.audio_arena_size = megabytes(256);
  ret.audio_samples_per_second = 48000;
  ret.audio_bits_per_sample = 16;
  ret.audio_channels = 2;
  
  ret.window_title = "tbg";

  ret.max_sprites = 4096;
  return ret;
}
