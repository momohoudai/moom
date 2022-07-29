/* date = June 8th 2022 6:27 pm */

#ifndef GAME_PAINTER_H
#define GAME_PAINTER_H

#include "momo_strings.h"
#include "game_assets.h"
#include "game_renderer.h"

struct Painter {
  Game_Assets* ga;
  Gfx_Command_Queue* cmds;
};


static void
begin_painting(Painter* p, 
               Game_Assets* ga, 
               Gfx_Command_Queue* cmds,
               F32 canvas_width,
               F32 canvas_height,
               U32 max_layers = 10000) 
{
  p->ga = ga;
  p->cmds = cmds;
  
  gfx_push_view(cmds, {}, canvas_width, canvas_height, max_layers);
}

static void
advance_depth(Painter* p) {
  gfx_advance_depth(p->cmds);
}

static void
paint_sprite(Painter* p,
             Sprite_ID sprite_id,
             V2 pos,
             V2 size,
             RGBA color = {1.f,1.f,1.f,1.f})
{
  Sprite_Asset* sprite = get_sprite(p->ga, sprite_id);
  Bitmap_Asset* bitmap = get_bitmap(p->ga, sprite->bitmap_id);
  V2 anchor = {0.5f, 0.5f}; 
  
  gfx_push_sprite(p->cmds, 
                  color,
                  pos, size, anchor,
                  bitmap->renderer_texture_handle, 
                  sprite->texel_uv);
}


static void
paint_text(Painter* p,
           Font_ID font_id,
           String str,
           RGBA color,
           F32 px, F32 py,
           F32 font_height) 
{
  Font_Asset* font = get_font(p->ga, font_id);
  Bitmap_Asset* bitmap = get_bitmap(p->ga, font->bitmap_id);
  for(U32 char_index = 0; 
      char_index < str.count;
      ++char_index) 
  {
    
    U32 curr_cp = str.e[char_index];
    if (char_index > 0) {
      U32 prev_cp = str.e[char_index-1];
      px += get_horizontal_advance(font, prev_cp, curr_cp)*font_height;
    }
    Font_Glyph_Asset *glyph = get_glyph(font, curr_cp);
    
    F32 width = (glyph->box.max.x - glyph->box.min.x)*font_height;
    F32 height = (glyph->box.max.y - glyph->box.min.y)*font_height;
    
    V2 pos = { px + (glyph->box.min.x*font_height), py + (glyph->box.min.y*font_height)};
    V2 size = { width, height };
    V2 anchor = {0.f, 0.f}; // bottom left
    gfx_push_sprite(p->cmds, 
                    color,
                    pos, size, anchor,
                    bitmap->renderer_texture_handle, 
                    glyph->texel_uv);
  }
  
}


static void
paint_line(Painter* p,
           Line2 line,
           F32 thickness,
           RGBA color = {1.f,1.f,1.f,1.f})
{
  gfx_push_line(p->cmds, 
                line, 
                thickness, 
                color); 
}


static void
paint_circle(Painter* p,
             Circ2 circle,
             F32 thickness, 
             U32 line_count,
             RGBA color) 
{
  gfx_push_circle(p->cmds, 
                  circle,
                  thickness,
                  line_count,
                  color);
}

static void
paint_triangle(Painter* p,
               RGBA colors,
               V2 p0, V2 p1, V2 p2) 
{
  gfx_push_triangle(p->cmds, 
                    colors,
                    p0, p1, p2);
}

static void
set_blend(Painter* p, Gfx_Blend_Type type) {
  gfx_push_blend(p->cmds, type);
}
#endif //GAME_PAINTER_H