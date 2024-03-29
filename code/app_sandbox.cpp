
#include "momo.h"
#include "eden_asset_id_sandbox.h"
#include "eden.h"



//
// Game functions
// 
exported 
eden_get_config_sig(eden_get_config) 
{
  eden_config_t ret;

  ret.target_frame_rate = 60;

  ret.max_inspector_entries = 8;
  ret.max_profiler_entries = 8;
  ret.max_profiler_snapshots = 120;

  ret.texture_queue_size = megabytes(5);
  ret.render_command_size = megabytes(100);
  ret.max_textures = 1;
  ret.max_texture_payloads = 1;
  ret.max_sprites = 4096;
  ret.max_triangles = 1;

  ret.audio_enabled = true;
  ret.audio_samples_per_second = 48000;
  ret.audio_bits_per_sample = 16;
  ret.audio_channels = 2;

  ret.window_title = "sandobokusu";
  ret.window_initial_width = 800;
  ret.window_initial_height = 800;

  return ret;
}


struct sandbox_t {
  arena_t arena;
};

exported 
eden_update_and_render_sig(eden_update_and_render) { 
  if (eden->user_data == nullptr)
  {
    eden->user_data = arena_bootstrap_push(sandbox_t, arena, megabytes(32)); 

    auto* sandbox = (sandbox_t*)(eden->user_data);
    eden_assets_init_from_file(eden, SANDBOX_ASSET_FILE, &sandbox->arena);

  }

  auto* sandbox = (sandbox_t*)(eden->user_data);


  eden_set_design_dimensions(eden, 1600, 900);
  eden_set_view(eden, -800.f, 800, -450.f, 450.f, 0.f, 0.f);
  eden_draw_asset_sprite(eden, ASSET_SPRITE_ID_BLANK, { 0.f, 0.f }, {100.f, 100.f});

}
