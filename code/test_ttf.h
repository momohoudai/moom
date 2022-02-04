/* date = January 31st 2022 0:39 pm */

#ifndef TEST_TTF_H
#define TEST_TTF_H

static U16
_ttf_read_u16(U8* location) {
  return endian_swap_16(*(U16*)location);
};

static S16
_ttf_read_s16(U8* location) {
  return endian_swap_16(*(U16*)location);
};
static U32
_ttf_read_u32(U8* location) {
  return endian_swap_32(*(U32*)location);
};



enum {
  _TTF_CMAP_PLATFORM_ID_UNICODE = 0,
  _TTF_CMAP_PLATFORM_ID_MACINTOSH = 1,
  _TTF_CMAP_PLATFORM_ID_RESERVED = 2,
  _TTF_CMAP_PLATFORM_ID_MICROSOFT = 3,
  
};

enum {
  _TTF_CMAP_MS_ID_SYMBOL = 0,
  _TTF_CMAP_MS_ID_UNICODE_BMP = 1,
  _TTF_CMAP_MS_ID_SHIFT_JIS = 2,
  _TTF_CMAP_MS_ID_PRC = 3,
  _TTF_CMAP_MS_ID_BIG_FIVE = 4,
  _TTF_CMAP_MS_ID_JOHAB = 5,
  _TTF_CMAP_MS_ID_UNICODE_FULL = 10,
  
};

struct TTF {
  U8* data;
  U32 glyph_count;
  
  // these are positions from data
  U32 loca, head, glyf, maxp, cmap, hhea;
  U32 cmap_mappings;
  
  U16 loca_format;
};



// This returns the scale you need to multiply to a font
// to get it's height in a certain pixel.
static F32
get_scale_for_pixel_height(TTF* ttf, F32 pixel_height) {
  S32 font_height = _ttf_read_s16(ttf->data + ttf->hhea + 4) - _ttf_read_s16(ttf->data + ttf->hhea + 6);
  return (F32)pixel_height/font_height;
}


// 0 is invalid
static U32
get_glyph_index_from_codepoint(TTF* ttf, U32 codepoint) {
  
  U16 format = _ttf_read_u16(ttf->data + ttf->cmap_mappings + 0);
  
  
  switch(format) {
    case 4: { // 
      U16 seg_count = _ttf_read_u16(ttf->data + ttf->cmap_mappings + 6) >> 1;
      U16 search_range = _ttf_read_u16(ttf->data + ttf->cmap_mappings + 8) >> 1;
      U16 entry_selector = _ttf_read_u16(ttf->data + ttf->cmap_mappings + 10);
      U16 range_shift = _ttf_read_u16(ttf->data + ttf->cmap_mappings + 12) >> 1;
      
      U32 end_codes = ttf->cmap_mappings + 14;
      U32 start_codes = end_codes + 2 + (2*seg_count);
      U32 id_deltas = start_codes + (2*seg_count);
      U32 id_range_offsets = id_deltas + (2*seg_count);
      U32 glyph_index_array = id_range_offsets + (2*seg_count);
      
      if (codepoint == 0xffff) return 0;
      
      // find the first end code that is greater than or equal to the codepoint
      // TODO: binary search?
      U16 seg_id = 0;
      U16 end_code = 0;
      for(U16 i = 0; i < seg_count; ++i) {
        end_code = _ttf_read_u16(ttf->data + end_codes + (2 * i));
        if( end_code >= codepoint ){
          seg_id = i;
          break;
        }
      }
      
      U16 start_code = _ttf_read_u16(ttf->data + start_codes + 2*seg_id);
      
      if (start_code > codepoint) return 0;
      
      U16 offset = _ttf_read_u16(ttf->data + id_range_offsets + 2*seg_id);
      S16 delta = _ttf_read_s16(ttf->data + id_deltas + 2*seg_id);
      
      if (offset == 0 ){
        return codepoint + delta;
      }
      else {
        return _ttf_read_u16(ttf->data +
                             id_range_offsets + 2*seg_id + // &id_range_offset[i]
                             offset + (codepoint - start_code)*2);
        
      }
      
    } break;
    
    default: {
      return 0; // invalid codepoint
    }
  }
}

static U32
_ttf_get_offset_to_glyph(TTF* ttf, U32 glyph_index) {
  assert(glyph_index < ttf->glyph_count);
  
  U32 g1 = 0, g2 = 0;
  switch(ttf->loca_format) {
    case 0: { // short format
      g1 = ttf->glyf + _ttf_read_u16(ttf->data + ttf->loca + glyph_index * 2) * 2;
      g2 = ttf->glyf + _ttf_read_u16(ttf->data + ttf->loca + glyph_index * 2 + 2) * 2;
    } break;
    case 1: { // long format
      g1 = ttf->glyf + _ttf_read_u16(ttf->data + ttf->loca + glyph_index * 4);
      g2 = ttf->glyf + _ttf_read_u16(ttf->data + ttf->loca + glyph_index * 4 + 4);
    } break;
    default: {
      return 0;
    }
  }
  
  return g1 == g2 ? 0 : g1;
  
}

// Get the glyph box as-is from the TTF.
//
// The glyph's box's coordinate system's origin is at the top right
// x moves towards the right, y moves towards the bottom
//
// ----x
// |
// |
// y
//
// The box contains values where:
//   min = bottom left of the glyph
//   max = top right of the glyph
// with respect to the coordinate system stated above.
// 
// TODO private?
static Rect2S
get_glyph_box(TTF* ttf, U32 glyph_index) {
  Rect2S ret = {};
  U32 g = _ttf_get_offset_to_glyph(ttf, glyph_index);
  
  ret.min.x = _ttf_read_s16(ttf->data + g + 2);
  ret.min.y = _ttf_read_s16(ttf->data + g + 4);
  ret.max.x = _ttf_read_s16(ttf->data + g + 6);
  ret.min.y = _ttf_read_s16(ttf->data + g + 8);
  
  return ret;
}



struct TTF_Glyph_Point {
  S16 x, y; 
  U8 flags;
  B32 is_end_point;
};


// A glyph point's coordinate system's origin is at the bottom left.
// x moves towards the right, y moves towards the top
//
// y
// |
// | 
// ----x
//
static Array<TTF_Glyph_Point>
get_glyph_points(TTF* ttf, U32 glyph_index, Arena* arena) {
  U32 g = _ttf_get_offset_to_glyph(ttf, glyph_index);
  S16 number_of_contours = _ttf_read_s16(ttf->data + g + 0);
  
  
  if (number_of_contours > 0) { // single glyph case
    U16 point_count = _ttf_read_u16(ttf->data + g + 10 + number_of_contours*2-2) + 1;
    U16 instruction_length = _ttf_read_u16(ttf->data + g + 10 + number_of_contours*2);
    
    U32 flags = g + 10 + number_of_contours*2 + 2 + instruction_length*2;
    
    // output end pts of contours
    //test_eval_d(number_of_contours);
    //test_eval_d(point_count);
    
    auto* points = push_array<TTF_Glyph_Point>(arena, point_count);
    zero_range(points, point_count); 
    U8* point_itr = ttf->data +  g + 10 + number_of_contours*2 + 2 + instruction_length*2;
    
    // Load the flags
    // flag info: https://docs.microsoft.com/en-us/typography/opentype/spec/glyf    
    {
      U8 current_flags = 0;
      U8 flag_count = 0;
      for (U32 i = 0; i < point_count; ++i) {
        if (flag_count == 0) {
          current_flags = *point_itr++;
          if (current_flags & 0x8) {
            flag_count = *point_itr++;
          }
        }
        else {
          --flag_count;
        }
        points[i].flags = current_flags;
      }
    }
    
    
    // Load the x coordinates
    {
      S16 x = 0;
      for (U32 i = 0; i < point_count; ++i ){
        flags = points[i].flags;
        if (flags & 0x2) {
          // if this is set, corresponding x-coordinate is 1 byte long
          // and the sign is determined by 0x10
          S16 dx = (S16)*point_itr++;
          x += (flags & 0x10) ? dx : -dx;            
        }
        else {
          // if this is not set, then...
          if (flags & 0x10) {
            // if this is set, then this x-coord is same as prev x-coord
            // i.e. we do nothing
          }
          else {
            // otherwise, this is 2 bytes long and intepreted as S16
            x += _ttf_read_s16(point_itr);
            point_itr += 2;
          }
          
        }
        points[i].x = x;
      }
    }
    
    // Load the y coordinates
    {
      S16 y = 0;
      for (U32 i = 0; i < point_count; ++i ){
        flags = points[i].flags;
        if (flags & 0x4) {
          // if this is set, corresponding y-coordinate is 1 byte long
          // and the sign is determined by 0x10
          S16 dy = (S16)*point_itr++;
          y += (flags & 0x20) ? dy : -dy;            
        }
        else {
          // if this is not set, then...
          if (flags & 0x20) {
            // if this is set, then this y-coord is same as prev y-coord
            // i.e. we do nothing
          }
          else {
            // otherwise, this is 2 bytes long and intepreted as S16
            y += _ttf_read_s16(point_itr);
            point_itr += 2;
          }
          
        }
        points[i].y = y;
      }
    }
    
    // mark the points that are contour endpoints
    {
      U32 contour_end_points = g + 10; 
      for (S16 i = 0; i < number_of_contours; ++i) {
        U16 contour_end_point_index = _ttf_read_u16(ttf->data + contour_end_points + i*2);
        points[contour_end_point_index].is_end_point = true;
      }
    }
    
    
    return create_array(points, point_count);
  }
  
  else if (number_of_contours < 0) { // compound glyph case
    test_log("compound glyph! %d\n", glyph_index);
    assert(false);
    return {};
  }
  else { //contour_count == 0
    // do nothing
    return {};
  } 
}

static TTF
read_ttf(Memory ttf_memory) {
  TTF ret = {};
  ret.data = ttf_memory.data_u8;
  
  U32 num_tables = _ttf_read_u16(ret.data + 4);
  
  for (U32 i= 0 ; i < num_tables; ++i ) {
    U32 directory = 12 + (16 * i);
    U32 tag = _ttf_read_u32(ret.data + directory + 0);
    
    test_create_log_section_until_scope;
    
    switch(tag) {
      case 'loca': {
        ret.loca = _ttf_read_u32(ret.data + directory + 8);
      }; break;
      case 'head': {
        ret.head = _ttf_read_u32(ret.data + directory + 8);
      }; break;
      case 'glyf': {
        ret.glyf = _ttf_read_u32(ret.data + directory + 8);
      }; break;
      case 'maxp': {
        ret.maxp = _ttf_read_u32(ret.data + directory + 8);
      } break;
      case 'cmap': {
        ret.cmap = _ttf_read_u32(ret.data + directory + 8);
      } break;
      case 'hhea': {
        ret.hhea = _ttf_read_u32(ret.data + directory + 8);
      } break;
      
    }
    
  }
  
  
  assert(ret.loca);
  assert(ret.maxp);
  assert(ret.head);
  assert(ret.glyf);
  assert(ret.cmap);
  assert(ret.hhea);
  
  ret.loca_format = _ttf_read_u16(ret.data + ret.head + 50);
  assert(ret.loca_format < 2);
  
  ret.glyph_count = _ttf_read_u16(ret.data + ret.maxp + 4);
  
  // Get index map
  {
    U32 subtable_count = _ttf_read_u16(ret.data + ret.cmap + 2);
    
    B32 found_index_table = false;
    
    for( U32 i = 0; i < subtable_count; ++i) {
      U32 subtable = ret.cmap + 4 + (8 * i);
      
      
      // We only support unicode encoding...
      // NOTE(Momo): They say mac is discouraged, so we won't care about it.
      U32 platform_id = _ttf_read_u16(ret.data + subtable + 0);
      switch(platform_id) {
        case _TTF_CMAP_PLATFORM_ID_MICROSOFT: {
          U32 platform_specific_id = _ttf_read_u16(ret.data + subtable + 2);
          switch(platform_specific_id) {
            case _TTF_CMAP_MS_ID_UNICODE_BMP:
            case _TTF_CMAP_MS_ID_UNICODE_FULL: {
              ret.cmap_mappings = ret.cmap + _ttf_read_u32(ret.data + subtable + 4);
              found_index_table =  true;
            }break;
            
          }
        }
        case _TTF_CMAP_PLATFORM_ID_UNICODE: {
          ret.cmap_mappings = ret.cmap + _ttf_read_u32(ret.data + subtable + 4);
          found_index_table = true;
        } break;
        
      }
      
      if (found_index_table) break;
    }
    
    assert(found_index_table && "unsupported cmap");
  }
  
  
  
  return ret;
}

struct TTF_Edge {
  V2 p0, p1;
  B32 is_inverted;
  F32 x_intersect;
};


static Image 
rasterize_codepoint(TTF* ttf, U32 codepoint, Arena* arena) {
  // TODO: scale param
  U32 glyph_index = get_glyph_index_from_codepoint(ttf, codepoint);
  F32 glyph_scale = get_scale_for_pixel_height(ttf, 128.f);
  
  // Scale the box and get the width and height of the box
  
  U32 image_width = 0;
  U32 image_height = 0;
  
  F32 width = 0;
  F32 height = 0;   
  {
    
    Rect2S raw_box = get_glyph_box(ttf, glyph_index);
    Rect2 box;
#if 0
    box.min.x = (S32)floor((F32)raw_box.min.x * glyph_scale);
    box.max.x = (S32)ceil((F32)raw_box.max.x * glyph_scale);
    box.min.y = (S32)floor((F32)raw_box.min.y * glyph_scale);
    box.max.y = (S32)ceil((F32)raw_box.max.y * glyph_scale);
#endif
    box.min.x = (F32)raw_box.min.x * glyph_scale;
    box.min.y = (F32)raw_box.max.y * glyph_scale;
    box.max.x = (F32)raw_box.max.x * glyph_scale;
    box.max.y = (F32)raw_box.min.y * glyph_scale;
    
    test_log("box for %d\n", glyph_index);
    {
      test_create_log_section_until_scope;
      test_eval_f(box.min.x);
      test_eval_f(box.max.x);
      test_eval_f(box.min.y);
      test_eval_f(box.max.y);
    }
    
    width = box.max.x - box.min.x;
    height = box.max.y - box.min.y;
    
    image_width = (U32)(box.max.x - box.min.x) + 1;
    image_height = (U32)(box.max.y - box.min.y) + 1;
    
  } 
  
  U32* pixels = push_array<U32>(arena, image_width * image_height);
  assert(pixels);
  
  // Set to white background
  // TODO(Momo): remove
  for (U32 i = 0; i < image_width*image_height; ++i) {
    pixels[i] = 0xFFFFFFFF;
  }
  
  create_scratch(scratch, arena);
  
  
  auto points = get_glyph_points(ttf, glyph_index, scratch);
  
  // generate scaled edges based on points
  Array<TTF_Edge> edges = {};
  {
    UMI edge_count = points.count;
    auto* e = push_array<TTF_Edge>(scratch, edge_count);
    assert(e);
    zero_range(e, edge_count);
    
    // NOTE(Momo): We have to scale the points and flip the y...
    UMI start_index = 0;
    B32 is_start = false;
    for (UMI i = 0; i < points.count; ++i) {
      if (is_start) {
        start_index = i;
        is_start = false;
      }
      
      
      e[i].p0.x = (F32)points.e[i].x * glyph_scale;
      e[i].p0.y = (F32)(height) - (F32)(points.e[i].y ) * glyph_scale;
      
      if (points.e[i].is_end_point) {
        is_start = true;
        e[i].p1.x = (F32)points.e[start_index].x * glyph_scale;
        e[i].p1.y = (F32)(height) - (F32)points.e[start_index].y * glyph_scale;
      }
      else {
        e[i].p1.x = (F32)points.e[i+1].x * glyph_scale;
        e[i].p1.y = (F32)(height) - (F32)points.e[i+1].y * glyph_scale;
      }
      
      // It's easier for the rasterization algorithm to have the edges'
      // p0 be on top of p1. If we flip, we will indicate it within the edge
      if (e[i].p0.y > e[i].p1.y) {
        swap(&e[i].p0, &e[i].p1);
        e[i].is_inverted = true;
      }
    }
    
    
    edges = create_array(e, points.count);
  }   
  
  
#if 1
  // NOTE(Momo): Debug
  for (UMI i = 0; i < edges.count; ++i) {
    auto* edge = edges.e + i;
    
    S32 sx = (S32)edge->p0.x;
    S32 sy = (S32)edge->p0.y;
    
    S32 ex = (S32)edge->p1.x;
    S32 ey = (S32)edge->p1.y;
    
    S32 dx = edge->p0.x < edge->p1.x ? 1 : -1;
    S32 dy = edge->p0.y < edge->p1.y ? 1 : -1;
    
    for(S32 x = sx; x != ex; x += dx)
      pixels[x + sy * image_width] = 0x000000FF;
    
    for(S32 y = sy; y != ey; y += dy)
      pixels[sx + y * image_width] = 0x000000FF;
    
  }
  
#endif
  
  
  // Rasterazation algo
  // Sort edges by top most edge
  quicksort(edges.e, edges.count, [](TTF_Edge* lhs, TTF_Edge* rhs) {
              F32 lhs_y = max_of(lhs->p0.y, lhs->p1.y);
              F32 rhs_y = max_of(rhs->p0.y, rhs->p1.y);
              return lhs_y < rhs_y;
            });
  
  // create an 'active edges list'
  auto active_edges = create_list(push_array<TTF_Edge*>(scratch, edges.count), edges.count);
  
  for (UMI i = 0; i < edges.count; ++i){
    auto* edge = edges.e + i;
    test_log("{%f %f} -> {%f %f}: %s\n",
             edge->p0.x,
             edge->p0.y,
             edge->p1.x,
             edge->p1.y,
             edge->is_inverted ? "inverted": "normal");
  }
  
  // NOTE(Momo): Currently, I'm lazy, so I'll just keep clearing and refilling the active_edges list per scan line
  //for(U32 line = 0; line < image_height; ++line) {
  for(U32 line = 0; line <= 0; ++line) {
    F32 linef = (F32)line + 0.5f; // 'center' of pixel
    clear(&active_edges);
    // Add to 'active edge list' any edges which have an uppermost vertex (p0) 
    // before this line and lowermost vertex after this line.
    for (UMI edge_index = 0; edge_index < edges.count; ++edge_index){
      auto* edge = edges.e + edge_index;
      if (edge->p0.y <= linef && edge->p1.y >= linef) {
        // calculate the x intersection
        F32 dx = edge->p1.x - edge->p0.x;
        F32 dy = edge->p1.y - edge->p0.y;
        if (dy != 0.f) {
          F32 t = (linef - edge->p0.y) / dy;
          edge->x_intersect = edge->p0.x + (t * dx);
          push_back(&active_edges, edge);
        }
      }
    }
    
    //sort the active edge list by their x_intersect
    quicksort(active_edges.e, active_edges.count, [](TTF_Edge**lhs, TTF_Edge** rhs) {
                return (*lhs)->x_intersect < (*rhs)->x_intersect;
              });
    
    
    
    test_log("checking current active edges\n");
    for (UMI i = 0; i < active_edges.count; ++i){
      auto* edge = active_edges.e[i];
      test_log("{%f %f} -> {%f %f} x %f: %s\n",
               edge->p0.x,
               edge->p0.y,
               edge->p1.x,
               edge->p1.y,
               edge->x_intersect,
               edge->is_inverted ? "inverted": "normal");
    }
    
    //TODO: time to draw!
  }
  
  
  
  
  
  
  
  
  
  
  
  
  Image ret;
  ret.width = image_width;
  ret.height = image_height;
  ret.pixels = pixels;
  
  return ret;
  
}

void test_ttf() {
  test_create_log_section_until_scope;
  
  U32 memory_size = MB(1);
  void * memory = malloc(memory_size);
  if (!memory) { 
    test_log("Cannot allocate memory");
    return;
  }
  defer { free(memory); };
  
  
  Arena main_arena = create_arena(memory, memory_size);
  Memory ttf_memory = test_read_file_to_memory(&main_arena, 
                                               test_assets_dir("nokiafc22.ttf"));
  
  TTF ttf = read_ttf(ttf_memory);
  
  U32 codepoint = 65;
  create_scratch(scratch, &main_arena);
  
  Image codepoint_image = rasterize_codepoint(&ttf, codepoint, scratch);
  {
    create_scratch(image_scratch, scratch);
    Memory image_mem = write_image_as_png(codepoint_image, image_scratch);
    test_write_memory_to_file(image_mem, "codepoint.png");
  }
  
  
}


#endif //TEST_TTF_H
