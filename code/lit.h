#ifndef MOE_MODE_LIT_H
#define MOE_MODE_LIT_H

#include "moe.h"

#define LIT_DEBUG_INTERSECTIONS 0
#define LIT_DEBUG_COORDINATES 0

#define LIT_WIDTH  800.f
#define LIT_HEIGHT 800.f

#define LIT_SENSOR_COLOR_MASK 0xFFFFFF00
#define LIT_SENSOR_RADIUS 8.f

#define LIT_SENSOR_PARTICLE_CD 0.1f
#define LIT_SENSOR_PARTICLE_SIZE 14.f
#define LIT_SENSOR_PARTICLE_SPEED 20.f

#define LIT_EXIT_FLASH_DURATION 0.1f
#define LIT_EXIT_FLASH_BRIGHTNESS 0.6f

#define LIT_ENTER_DURATION 3.f

#define LIT_PLAYER_RADIUS 16.f
#define LIT_PLAYER_LIGHT_RETRIEVE_DURATION 0.05f
#define LIT_PLAYER_BREATH_DURATION 2.f
#define LIT_PLAYER_PICKUP_DIST 512.f
#define LIT_PLAYER_ROTATE_SPEED 1.f


#ifdef INTERNAL
#define lit_log(...) lit->platform->debug_log(__VA_ARGS__)
#define lit_profile_block(name) profiler_block(moe->platform->profiler, name)
#define lit_profile_begin(name) profiler_begin_block(moe->platform->profiler, name)
#define lit_profile_end(name) profiler_end_block(moe->platform->profiler, name)
#else
#define lit_log(...)
#define lit_profiler_block(...)
#define lit_profile_begin(...) 
#define lit_profile_end(...) 
#endif 



#include "lit_splash.h"
#include "lit_menu.h"
#include "lit_game.h"

enum lit_show_debug_type_t {
  LIT_SHOW_DEBUG_NONE,
  LIT_SHOW_DEBUG_PROFILER,
  LIT_SHOW_DEBUG_CONSOLE,
  LIT_SHOW_DEBUG_INSPECTOR,
  
  LIT_SHOW_DEBUG_MAX
};

enum lit_mode_t {
  LIT_MODE_SPLASH,
  LIT_MODE_MENU,
  LIT_MODE_GAME,
};

struct lit_save_data_t {
  // TODO
};

struct lit_t {
  moe_t* moe;
  gfx_t* gfx; 
  profiler_t* profiler;
  input_t* input;

  lit_show_debug_type_t show_debug_type;
  lit_mode_t next_mode;
  lit_mode_t mode;
  union {
    lit_splash_t splash;
    lit_game_t game;
    lit_menu_t menu;
  };
  u32_t level_to_start;

  //
  // Arenas
  //
  arena_t main_arena;
  arena_t asset_arena;
  arena_t debug_arena;
  arena_t frame_arena;

  // 
  // Debug Tools
  //
  inspector_t inspector;
  console_t console;

  assets_t assets;
  asset_sprite_id_t blank_sprite;
  asset_font_id_t debug_font;
};

static void 
lit_goto_specific_level(lit_t* lit, u32_t level) {
  //assert(level < array_count(g_lit_levels));
  lit->next_mode = LIT_MODE_GAME;
  lit->level_to_start = level;
}

#include "lit_splash.cpp"
#include "lit_menu.cpp"
#include "lit_game.cpp"

#include "lit_console_rendering.h"
#include "lit_profiler_rendering.h"
#include "lit_inspector_rendering.h"
#include "lit_levels.h"



#endif 
