#ifndef MOMO_SHAPES_H
#define MOMO_SHAPES_H

//~ Rects
struct Rect2F32 {
  V2F32 min, max;
};

struct Rect2U32 {
  V2U32 min, max;
};

struct Rect3F32{
  V3F32 min, max;
};


static constexpr F32 Width(Rect2F32);
static constexpr U32 Width(Rect2U32);
static constexpr F32 Width(Rect3F32);

static constexpr F32 Height(Rect2F32);
static constexpr U32 Height(Rect2U32);
static constexpr F32 Height(Rect3F32);

static constexpr F32 Depth(Rect3F32);


//~ Circles
// NOTE(Momo): I don't think I have any use case for
// circles that are not F32, so I'm not going to suffix 
// it with a type
struct Circ2
{
  F32 radius;
  V2F32 center;
};


//~ NOTE(Momo): Lines
struct Line2 {
  V2F32 min, max;
};


#include "momo_shapes.cpp"

#endif //MOMO_SHAPES_H
