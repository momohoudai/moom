


#ifndef GAME_H
#define GAME_H

#include "game_gfx.h"
#include "game_console.h"
#include "game_asset_file.h"


//
// Profiler 
// 
typedef u64_t game_profiler_get_performance_counter_f();

struct game_profiler_snapshot_t {
  u32_t hits;
  u32_t cycles;
};

struct game_profiler_entry_t {
  u32_t line;
  const char* filename;
  const char* block_name;
  u64_t hits_and_cycles;
  
  game_profiler_snapshot_t* snapshots;
  
  // NOTE(Momo): For initialization of entry. 
  // Maybe it shouldn't be stored here
  // but on where they called it? 
  // i.e. use a functor that wraps?
  u32_t start_cycles;
  u32_t start_hits;
  b32_t flag_for_reset;
};


struct game_profiler_t {
  u32_t entry_snapshot_count;
  u32_t entry_count;
  u32_t entry_cap;
  game_profiler_entry_t* entries;
  u32_t snapshot_index;

  game_profiler_get_performance_counter_f* get_performance_counter;
};

#define game_profiler_begin_block(p, name) \
  static game_profiler_entry_t* _profiler_block_##name = 0; \
  if (_profiler_block_##name == 0 || _profiler_block_##name->flag_for_reset) {\
    _profiler_block_##name = _game_profiler_init_block(p, __FILE__, __LINE__, __FUNCTION__, #name);  \
  }\
  _game_profiler_begin_block(p, _profiler_block_##name)\

#define game_profiler_end_block(p, name) \
  _game_profiler_end_block(p, _profiler_block_##name) 

#define game_profiler_block(p, name) game_profiler_begin_block(p, name); defer {game_profiler_end_block(p,name);}

// Correspond with API
#define game_profile_begin(game, name) game_profiler_begin_block(&game->profiler, name)
#define game_profile_end(game, name)   game_profiler_end_block(&game->profiler, name)
#define game_profile_block(game, name) game_profiler_block(&game->profiler, name)


//
// Inspector 
//
enum game_inspector_entry_type_t {
  GAME_INSPECTOR_ENTRY_TYPE_F32,
  GAME_INSPECTOR_ENTRY_TYPE_U32,
};

struct game_inspector_entry_t {
  st8_t name;
  game_inspector_entry_type_t type;
  union {
    f32_t item_f32;
    u32_t item_u32;
  };
};

struct game_inspector_t {
  u32_t entry_cap;
  u32_t entry_count;
  game_inspector_entry_t* entries;
};

// API correspondence
#define game_inspect_u32(game, name, item) game_inspector_add_u32(&game->inspector, name, item)
#define game_inspect_f32(game, name, item) game_inspector_add_f32(&game->inspector, name, item)

// 
// Game
//
enum game_blend_type_t {
  GAME_BLEND_TYPE_ZERO,
  GAME_BLEND_TYPE_ONE,
  GAME_BLEND_TYPE_SRC_COLOR,
  GAME_BLEND_TYPE_INV_SRC_COLOR,
  GAME_BLEND_TYPE_SRC_ALPHA,
  GAME_BLEND_TYPE_INV_SRC_ALPHA,
  GAME_BLEND_TYPE_DST_ALPHA,
  GAME_BLEND_TYPE_INV_DST_ALPHA,
  GAME_BLEND_TYPE_DST_COLOR,
  GAME_BLEND_TYPE_INV_DST_COLOR,
};

#define game_set_view_sig(name) void name(f32_t min_x, f32_t max_x, f32_t min_y, f32_t max_y, f32_t pos_x, f32_t pos_y)
typedef game_set_view_sig(game_set_view_f);
#define game_set_view(game, ...) (game->set_view(__VA_ARGS__))

#define game_clear_canvas_sig(name) void name(rgba_t color)
typedef game_clear_canvas_sig(game_clear_canvas_f);
#define game_clear_canvas(game, ...) (game->clear_canvas(__VA_ARGS__))

#define game_draw_sprite_sig(name) void name(v2f_t pos, v2f_t size, v2f_t anchor, u32_t texture_index, u32_t texel_x0, u32_t texel_y0, u32_t texel_x1, u32_t texel_y1, rgba_t color)
typedef game_draw_sprite_sig(game_draw_sprite_f);
#define game_draw_sprite(game, ...) (game->draw_sprite(__VA_ARGS__))

#define game_draw_rect_sig(name) void name(v2f_t pos, f32_t rot, v2f_t scale, rgba_t color)
typedef game_draw_rect_sig(game_draw_rect_f);
#define game_draw_rect(game, ...) (game->draw_rect(__VA_ARGS__))

#define game_draw_tri_sig(name) void name(v2f_t p0, v2f_t p1, v2f_t p2, rgba_t color)
typedef game_draw_tri_sig(game_draw_tri_f);
#define game_draw_tri(game, ...) (game->draw_tri(__VA_ARGS__))

#define game_advance_depth_sig(name) void name(void)
typedef game_advance_depth_sig(game_advance_depth_f);
#define game_advance_depth(game, ...) (game->advance_depth(__VA_ARGS__))

#define game_set_blend_sig(name) void name(game_blend_type_t src, game_blend_type_t dst)
typedef game_set_blend_sig(game_set_blend_f);
#define game_set_blend(game, ...) (game->set_blend(__VA_ARGS__))


// 
// Button input
//
struct game_button_t {
  b32_t before : 1;
  b32_t now: 1; 
};

enum game_button_code_t {
  // my god
  // Keyboard keys
  GAME_BUTTON_CODE_UNKNOWN,
  GAME_BUTTON_CODE_0,
  GAME_BUTTON_CODE_1,
  GAME_BUTTON_CODE_2,
  GAME_BUTTON_CODE_3,
  GAME_BUTTON_CODE_4,
  GAME_BUTTON_CODE_5,
  GAME_BUTTON_CODE_6,
  GAME_BUTTON_CODE_7,
  GAME_BUTTON_CODE_8,
  GAME_BUTTON_CODE_9,
  GAME_BUTTON_CODE_F1,
  GAME_BUTTON_CODE_F2,
  GAME_BUTTON_CODE_F3,
  GAME_BUTTON_CODE_F4,
  GAME_BUTTON_CODE_F5,
  GAME_BUTTON_CODE_F6,
  GAME_BUTTON_CODE_F7,
  GAME_BUTTON_CODE_F8,
  GAME_BUTTON_CODE_F9,
  GAME_BUTTON_CODE_F10,
  GAME_BUTTON_CODE_F11,
  GAME_BUTTON_CODE_F12,
  GAME_BUTTON_CODE_BACKSPACE,
  GAME_BUTTON_CODE_A,
  GAME_BUTTON_CODE_B,
  GAME_BUTTON_CODE_C,
  GAME_BUTTON_CODE_D,
  GAME_BUTTON_CODE_E,
  GAME_BUTTON_CODE_F,
  GAME_BUTTON_CODE_G,
  GAME_BUTTON_CODE_H,
  GAME_BUTTON_CODE_I,
  GAME_BUTTON_CODE_J,
  GAME_BUTTON_CODE_K,
  GAME_BUTTON_CODE_L,
  GAME_BUTTON_CODE_M,
  GAME_BUTTON_CODE_N,
  GAME_BUTTON_CODE_O,
  GAME_BUTTON_CODE_P,
  GAME_BUTTON_CODE_Q,
  GAME_BUTTON_CODE_R,
  GAME_BUTTON_CODE_S,
  GAME_BUTTON_CODE_T,
  GAME_BUTTON_CODE_U,
  GAME_BUTTON_CODE_V,
  GAME_BUTTON_CODE_W,
  GAME_BUTTON_CODE_X,
  GAME_BUTTON_CODE_Y,
  GAME_BUTTON_CODE_Z,
  GAME_BUTTON_CODE_SPACE,
  GAME_BUTTON_CODE_RMB,
  GAME_BUTTON_CODE_LMB,
  GAME_BUTTON_CODE_MMB,

  GAME_BUTTON_CODE_MAX,

};

//
// App Input API
//
// NOTE(momo): Input is SPECIFICALLY stuff that can be recorded and
// replayed by some kind of system. Other things go to game_t
// 
struct game_input_characters_t {
  u8_t* data;
  u32_t count;
};

struct game_input_t {
  game_button_t buttons[GAME_BUTTON_CODE_MAX];
  u8_t chars[32];
  u32_t char_count;

  // NOTE(Momo): The mouse position is relative to the moe's dimensions given
  // via set_design_dims(). It is possible to get back the normalized dimensions
  // by dividing the x/y by the width/height of the moe.
  v2f_t mouse_pos;

  // NOTE(Momo): Mouse wheels values are not normally analog!
  // +ve is forwards
  // -ve is backwards
  s32_t mouse_scroll_delta;

  // TODO(Momo): not sure if this should even be here
  f32_t delta_time; //aka dt
};

//
// File IO API
// 
enum game_file_path_t {
  GAME_FILE_PATH_EXE,
  GAME_FILE_PATH_USER,
  GAME_FILE_PATH_CACHE,

};

enum game_file_access_t {
  GAME_FILE_ACCESS_READ,
  GAME_FILE_ACCESS_OVERWRITE,
};

struct game_file_t {
  void* data; // pointer for platform's usage
};

#define game_open_file_sig(name) b32_t name(game_file_t* file, const char* filename, game_file_access_t file_access, game_file_path_t file_path)
typedef game_open_file_sig(game_open_file_f);
#define game_open_file(game, ...) (game->open_file(__VA_ARGS__))

#define game_close_file_sig(name) void  name(game_file_t* file)
typedef game_close_file_sig(game_close_file_f);
#define game_close_file(game, ...) (game->close_file(__VA_ARGS__))

#define game_read_file_sig(name) b32_t name(game_file_t* file, usz_t size, usz_t offset, void* dest)
typedef game_read_file_sig(game_read_file_f);
#define game_read_file(game, ...) (game->read_file(__VA_ARGS__))

#define game_write_file_sig(name) b32_t name(game_file_t* file, usz_t size, usz_t offset, void* src)
typedef game_write_file_sig(game_write_file_f);
#define game_write_file(game, ...) (game->write_file(__VA_ARGS__))

#define game_get_file_size_sig(name) u64_t name(game_file_t* file)
typedef game_get_file_size_sig(game_get_file_size_f);
#define game_get_file_size(game, ...) (game->get_file_size(__VA_ARGS__))


//
// App Logging API
// 
#define game_debug_log_sig(name) void name(const char* fmt, ...)
typedef game_debug_log_sig(game_debug_log_f);
#define game_debug_log(game, ...) (game->debug_log(__VA_ARGS__))

//
// App Cursor API
//
#define game_show_cursor_sig(name) void name()
typedef game_show_cursor_sig(game_show_cursor_f);
#define game_show_cursor(game, ...) (game->show_cursor(__VA_ARGS__))

#define game_hide_cursor_sig(name) void name()
typedef game_hide_cursor_sig(game_hide_cursor_f);
#define game_hide_cursor(game, ...) (game->hide_cursor(__VA_ARGS__))

#define game_lock_cursor_sig(name) void name()
typedef game_lock_cursor_sig(game_lock_cursor_f);
#define game_lock_cursor(game, ...) (game->lock_cursor(__VA_ARGS__))

#define game_unlock_cursor_sig(name) void name()
typedef game_unlock_cursor_sig(game_unlock_cursor_f);
#define game_unlock_cursor(game, ...) (game->unlock_cursor(__VA_ARGS__))


//
// Memory Allocation API
//
#define game_allocate_memory_sig(name) void* name(usz_t size)
typedef game_allocate_memory_sig(game_allocate_memory_f);
#define game_allocate_memory(game, ...) (game->allocate_memory(__VA_ARGS__))

#define game_free_memory_sig(name) void name(void* ptr)
typedef game_free_memory_sig(game_free_memory_f);
#define game_free_memory(game, ...) (game->free_memory(__VA_ARGS__))

//
// Multithreaded work API
//
typedef void game_task_callback_f(void* data);

#define game_add_task_sig(name) void name(game_task_callback_f callback, void* data)
typedef game_add_task_sig(game_add_task_f);
#define game_add_task(game, ...) (game->add_task(__VA_ARGS__))

#define game_complete_all_tasks_sig(name) void name(void)
typedef game_complete_all_tasks_sig(game_complete_all_tasks_f);
#define game_complete_all_tasks(game, ...) (game->complete_all_tasks(__VA_ARGS__))

// 
// Window/Graphics related
//
#define game_set_design_dimensions_sig(name) void name(f32_t width, f32_t height)
typedef game_set_design_dimensions_sig(game_set_design_dimensions_f);
#define game_set_design_dimensions(game, ...) (game->set_design_dimensions(__VA_ARGS__))


//
// App Audio API
//
struct game_audio_t {
  s16_t* sample_buffer;
  u32_t sample_count;
  u32_t channels; //TODO: remove this?
  
  void* platform_data;
};


//
// Arena Stat
//

struct game_t {
  game_show_cursor_f* show_cursor;
  game_hide_cursor_f* hide_cursor;
  game_lock_cursor_f* lock_cursor;
  game_unlock_cursor_f* unlock_cursor;
  game_allocate_memory_f* allocate_memory;
  game_free_memory_f* free_memory;
  game_debug_log_f* debug_log;
  game_add_task_f* add_task;
  game_complete_all_tasks_f* complete_all_tasks;
  game_set_design_dimensions_f* set_design_dimensions;
  game_open_file_f* open_file;
  game_close_file_f* close_file;
  game_write_file_f* write_file;
  game_read_file_f* read_file;
  game_set_view_f* set_view;
  game_clear_canvas_f* clear_canvas;
  game_draw_sprite_f* draw_sprite;
  game_draw_rect_f* draw_rect;
  game_draw_tri_f* draw_tri;
  game_advance_depth_f* advance_depth;
  game_get_file_size_f* get_file_size;

  game_input_t input;
  game_audio_t audio; 

  gfx_t* gfx;
  game_profiler_t profiler;
  game_inspector_t inspector;
          
  b32_t is_dll_reloaded;
  b32_t is_running;

  // arenas relevant to the game
  arena_t gfx_arena;
  arena_t audio_arena;
  arena_t debug_arena;

  void* game;
};


////////////////////////////////////
// 
// Game API
//
//
struct game_init_config_t {

  usz_t debug_arena_size;
  u32_t max_inspector_entries;
  u32_t max_profiler_entries;
  u32_t max_profiler_snapshots; // snapshots per entry


  usz_t gfx_arena_size;
  usz_t texture_queue_size;
  usz_t render_command_size;
  u32_t max_textures;

  b32_t audio_enabled;
  usz_t audio_arena_size;

  // must be null terminated
  const char* window_title; 
};

#define game_init_sig(name) game_init_config_t name(void)
typedef game_init_sig(game_init_f);

#define game_update_and_render_sig(name) void name(game_t* game)
typedef game_update_and_render_sig(game_update_and_render_f);

// To be called by platform
struct game_functions_t {
  game_init_f* init;
  game_update_and_render_f* update_and_render;
};

static const char* game_function_names[] {
  "game_init",
  "game_update_and_render",
};

#include "game_assets.h"

///////////////////////////////
///
// IMPLEMENTATIONS
//
//

// before: 0, now: 1
static b32_t
game_is_button_poked(game_t* game, game_button_code_t code) {
  game_input_t* in = &game->input;
  auto btn = in->buttons[code];
  return !btn.before && btn.now;
}

// before: 1, now: 0
static b32_t
game_is_button_released(game_t* game, game_button_code_t code) {
  game_input_t* in = &game->input;
  auto btn = in->buttons[code];
  return btn.before && !btn.now;
}

// before: X, now: 1
static b32_t
game_is_button_down(game_t* game, game_button_code_t code){
  game_input_t* in = &game->input;
  return in->buttons[code].now;
}


// before: 1, now: 1
static b32_t
game_is_button_held(game_t* game, game_button_code_t code) {
  game_input_t* in = &game->input;
  auto btn = in->buttons[code];
  return btn.before && btn.now;
}

static b32_t
game_is_dll_reloaded(game_t* game) {
  return game->is_dll_reloaded;
}

static f32_t 
game_get_dt(game_t* game) {
  return game->input.delta_time;
}


static game_input_characters_t
game_get_input_characters(game_t* game) {
  game_input_characters_t ret;
  ret.data = game->input.chars;
  ret.count = game->input.char_count; 

  return ret;
}

//
// Deriviative Graphics API functions
//
static void
game_set_blend_additive(game_t* game) {
  gfx_set_blend_additive(game->gfx);
}

static void
game_set_blend_alpha(game_t* game) {
  gfx_set_blend_alpha(game->gfx);
}

static void
game_draw_line(game_t* game, v2f_t p0, v2f_t p1, f32_t thickness, rgba_t colors) {
  gfx_draw_line(game->gfx, p0, p1, thickness, colors);
}

static void
game_draw_circle(game_t* game, v2f_t center, f32_t radius, u32_t sections, rgba_t color) {
  gfx_draw_filled_circle(game->gfx, center, radius, sections, color);
}

static void
game_draw_circ_outline(game_t* game, v2f_t center, f32_t radius, f32_t thickness, u32_t line_count, rgba_t color) 
{
  gfx_draw_circle_outline(game->gfx, center, radius, thickness, line_count, color);
}


static void
game_draw_asset_sprite(
    game_t* game, 
    assets_t* assets, 
    asset_sprite_id_t sprite_id, 
    v2f_t pos, 
    v2f_t size, 
    rgba_t color = rgba_set(1.f,1.f,1.f,1.f))
{
  asset_sprite_t* sprite = assets_get_sprite(assets, sprite_id);
  asset_bitmap_t* bitmap = assets_get_bitmap(assets, sprite->bitmap_asset_id);
  v2f_t anchor = v2f_set(0.5f, 0.5f); 
  
  game_draw_sprite(
      game, 
      pos, size, anchor,
      bitmap->renderer_texture_handle, 
      sprite->texel_x0,
      sprite->texel_y0,
      sprite->texel_x1,
      sprite->texel_y1,
      color);
}


static void
game_draw_text(game_t* game, assets_t* assets, asset_font_id_t font_id, st8_t str, rgba_t color, f32_t px, f32_t py, f32_t font_height) 
{
  asset_font_t* font = assets_get_font(assets, font_id);
  for(u32_t char_index = 0; 
      char_index < str.count;
      ++char_index) 
  {
    u32_t curr_cp = str.e[char_index];

    if (char_index > 0) {
      u32_t prev_cp = str.e[char_index-1];
      asset_font_glyph_t *prev_glyph = assets_get_glyph(font, prev_cp);

      f32_t kerning = assets_get_kerning(font, prev_cp, curr_cp);
      f32_t advance = prev_glyph->horizontal_advance;
      px += (kerning + advance) * font_height;
    }

    asset_font_glyph_t *glyph = assets_get_glyph(font, curr_cp);
    asset_bitmap_t* bitmap = assets_get_bitmap(assets, font->bitmap_asset_id);
    f32_t width = (glyph->box_x1 - glyph->box_x0)*font_height;
    f32_t height = (glyph->box_y1 - glyph->box_y0)*font_height;
    
    v2f_t pos = v2f_set(px + (glyph->box_x0*font_height), py + (glyph->box_y0*font_height));
    v2f_t size = v2f_set(width, height);
    v2f_t anchor = v2f_set(0.f, 0.f); // bottom left
    game_draw_sprite(game, 
                    pos, size, anchor,
                    bitmap->renderer_texture_handle, 
                    glyph->texel_x0,
                    glyph->texel_y0,
                    glyph->texel_x1,
                    glyph->texel_y1,
                    color);
  }
  
}

static void
game_draw_text_center_aligned(game_t* game, assets_t* assets, asset_font_id_t font_id, st8_t str, rgba_t color, f32_t px, f32_t py, f32_t font_height) 
{
  asset_font_t* font = assets_get_font(assets, font_id);
  
  // Calculate the total width of the text
  f32_t offset = 0.f;
  for(u32_t char_index = 1; 
      char_index < str.count;
      ++char_index)
  {

    u32_t curr_cp = str.e[char_index];
    u32_t prev_cp = str.e[char_index-1];

    asset_font_glyph_t *prev_glyph = assets_get_glyph(font, prev_cp);
    asset_font_glyph_t *curr_glyph = assets_get_glyph(font, curr_cp);

    f32_t kerning = assets_get_kerning(font, prev_cp, curr_cp);
    f32_t advance = prev_glyph->horizontal_advance;
    offset += (kerning + advance) * font_height;
  }

  // Add the width of the last glyph
  {    
    u32_t cp = str.e[str.count-1];
    asset_font_glyph_t* glyph = assets_get_glyph(font, cp);
    f32_t advance = glyph->horizontal_advance;
    offset += advance * font_height;
  }
  px -= offset/2 ;

  for(u32_t char_index = 0; 
      char_index < str.count;
      ++char_index) 
  {
    u32_t curr_cp = str.e[char_index];

    if (char_index > 0) {
      u32_t prev_cp = str.e[char_index-1];
      asset_font_glyph_t *prev_glyph = assets_get_glyph(font, prev_cp);

      f32_t kerning = assets_get_kerning(font, prev_cp, curr_cp);
      f32_t advance = prev_glyph->horizontal_advance;
      px += (kerning + advance) * font_height;
    }

    asset_font_glyph_t *glyph = assets_get_glyph(font, curr_cp);
    asset_bitmap_t* bitmap = assets_get_bitmap(assets, font->bitmap_asset_id);
    f32_t width = (glyph->box_x1 - glyph->box_x0)*font_height;
    f32_t height = (glyph->box_y1 - glyph->box_y0)*font_height;
    
    v2f_t pos = v2f_set(px + (glyph->box_x0*font_height), py + (glyph->box_y0*font_height));
    v2f_t size = v2f_set(width, height);
    v2f_t anchor = v2f_set(0.f, 0.f); // bottom left
    game_draw_sprite(game, 
                    pos, size, anchor,
                    bitmap->renderer_texture_handle, 
                    glyph->texel_x0,
                    glyph->texel_y0,
                    glyph->texel_x1,
                    glyph->texel_y1,
                    color
                    );
  }

}

static void 
game_inspector_init(game_inspector_t* in, arena_t* arena, u32_t max_entries) 
{
  in->entry_cap = max_entries;
  in->entry_count = 0;
  in->entries = arena_push_arr(game_inspector_entry_t, arena, max_entries);
  assert(in->entries != nullptr);
}

static void 
game_inspector_clear(game_inspector_t* in) 
{
  in->entry_count = 0;
}

static void
game_inspector_add_u32(game_inspector_t* in, st8_t name, u32_t item) 
{
  assert(in->entry_count < in->entry_cap);
  game_inspector_entry_t* entry = in->entries + in->entry_count++;
  entry->item_u32 = item;
  entry->type = GAME_INSPECTOR_ENTRY_TYPE_U32;
  entry->name = name;
}


static void
game_inspector_add_f32(game_inspector_t* in, st8_t name, f32_t item) {
  assert(in->entry_count < in->entry_cap);
  game_inspector_entry_t* entry = in->entries + in->entry_count++;
  entry->item_f32 = item;
  entry->type = GAME_INSPECTOR_ENTRY_TYPE_F32;
  entry->name = name;
}

static game_profiler_entry_t*
_game_profiler_init_block(
    game_profiler_t* p,
    const char* filename, 
    u32_t line,
    const char* function_name,
    const char* block_name = 0) 
{
  if (p->entry_count < p->entry_cap) {
    game_profiler_entry_t* entry = p->entries + p->entry_count++;
    entry->filename = filename;
    entry->block_name = block_name ? block_name : function_name;
    entry->line = line;
    entry->start_cycles = (u32_t)p->get_performance_counter();
    entry->start_hits = 1;
    entry->flag_for_reset = false;
    return entry;
  }

  return nullptr;
}

static void
_game_profiler_begin_block(game_profiler_t* p, game_profiler_entry_t* entry) 
{
  entry->start_cycles = (u32_t)p->get_performance_counter();
  entry->start_hits = 1;
}

static void
_game_profiler_end_block(game_profiler_t* p, game_profiler_entry_t* entry) {
  u64_t delta = ((u32_t)p->get_performance_counter() - entry->start_cycles) | ((u64_t)(entry->start_hits)) << 32;
  u64_atomic_add(&entry->hits_and_cycles, delta);
}


static void 
game_profiler_reset(game_profiler_t* p) {

  for(u32_t entry_id = 0; entry_id < p->entry_count; ++entry_id)
  {
    game_profiler_entry_t* itr = p->entries + entry_id;
    itr->flag_for_reset = true;
  }

  p->entry_count = 0;
}

static void 
game_profiler_init(
    game_profiler_t* p, 
    game_profiler_get_performance_counter_f* get_performance_counter,
    arena_t* arena,
    u32_t max_entries,
    u32_t max_snapshots_per_entry)
{
  p->entry_cap = max_entries;
  p->entry_snapshot_count = max_snapshots_per_entry;
  p->entries = arena_push_arr(game_profiler_entry_t, arena, p->entry_cap);
  assert(p->entries);
  p->get_performance_counter = get_performance_counter;

  for (u32_t i = 0; i < p->entry_cap; ++i) {
    p->entries[i].snapshots = arena_push_arr(game_profiler_snapshot_t, arena, max_snapshots_per_entry);
    assert(p->entries[i].snapshots);
  }
  game_profiler_reset(p);
}


static void
game_profiler_update_entries(game_profiler_t* p) {
  for(u32_t entry_id = 0; entry_id < p->entry_count; ++entry_id)
  {
    game_profiler_entry_t* itr = p->entries + entry_id;
    u64_t hits_and_cycles = u64_atomic_assign(&itr->hits_and_cycles, 0);
    u32_t hits = (u32_t)(hits_and_cycles >> 32);
    u32_t cycles = (u32_t)(hits_and_cycles & 0xFFFFFFFF);
    
    itr->snapshots[p->snapshot_index].hits = hits;
    itr->snapshots[p->snapshot_index].cycles = cycles;
  }
  ++p->snapshot_index;
  if(p->snapshot_index >= p->entry_snapshot_count) {
    p->snapshot_index = 0;
  }
}

#endif //GAME_H


//
// JOURNAL
//
// = 2023-08-10 =
//   I spent an afternoon yesterday thinking how I could remove things
//   like the config from the game layer because a part of me believes
//   that the game layer shouldn't know the specifics of the engine layer
//   like who many bytes should the "graphics arena" have. 
//
//   At the same time, if the game layer doesn't specify, the engine because 
//   too general purpose, and that would require me to write a bunch of general
//   purpose stuff (like a general purpose allocator) which...could have really
//   inefficient outcomes if the stars do not align, like higher wastage of memory.
//   
//   Thus this is a reminder to myself to spearhead and go with the config idea.
//   The next thing to do is for the game to somehow retrieve the arena usages 
//   of the engine so that the game side can manually fine tune their numbers.
// 
// = 2023-07-30 = 
//   I'm not entirely sure where assets should really be.
//   I feel like they should be shifted *somewhere* but it's hard
//   to figure out exactly where. The main issue I *feel* is that
//   the game side shouldn't be the one to initialize the assets;
//   instead it should be on the game's side. This would make it 
//   more reasonable to do some kind of 'hot reloading' of assets.
//
// = 2023-07-18 = 
//   We probably should start working on either the gfx layer or the
//   profiler/debug layer next.
//
//   For the gfx layer, we will probably want to remove the need for
//   a command buffer and maybe even the texture buffer, at least 
//   from the views of the game layer. 
//
//   Profiler layer is more straightforward...it's more of whether we
//   should consolidate ALL debug-related things into one big struct. 
