#ifndef MOMO_SHAPES_H
#define MOMO_SHAPES_H


typedef struct Rect2 {
  V2 min, max;
}Rect2;

typedef struct Rect2S {
  V2S min, max;
}Rect2S;

typedef struct Rect2U {
  V2U min, max;
}Rect2U;

typedef struct Rect3 {
  V3 min, max;
}Rect3;

typedef struct Aabb2 {
  V2 anchor;
  V2 dims;
}Aabb2;

typedef struct Circ2 {
  F32 radius;
  V2 center;
}Circ2;


typedef struct Line2 {
  V2 min, max;
} Line2;

typedef struct Ray2 {
  V2 pt;
  V2 dir;
} Ray2;

typedef struct Tri2 {
  V2 pts[3];
} Tri2;

static Line2 line2_set(V2 min, V2 max);
static Tri2  tri2_set(V2 pt0, V2 pt1, V2 p2);
static Circ2 circ2_set(V2 center, F32 radius);

static B32 bonk_circ2_circ2_set(Circ2 a, Circ2 b);
static B32 bonk_line2_pt2(Line2, V2 pt);

///////////////////////////////////////////////////////////////////
// IMPLEMENTATION
static Circ2
circ2_set(V2 center, F32 radius) {
  Circ2 ret = {0};
  ret.center = center;
  ret.radius = radius;
  return ret;
}
static Line2
line2_set(V2 min, V2 max) {
  Line2 ret = {0};
  ret.min = min;
  ret.max = max;
  return ret;
}

static Tri2
tri2_set(V2 pt0, V2 pt1, V2 pt2) {
  Tri2 ret = { pt0, pt1, pt2 };
  return ret;
}



// https://totologic.blogspot.com/2014/01/accurate-point-in-triangle-test.html
// NOTE(Momo): We should really profile to see which is the best but I'm assuming
// it's the dot product one
static B32
_bonk_tri2_pt2_parametric(Tri2 tri, V2 pt) {
  F32 denominator = (tri.pts[0].x*(tri.pts[1].y - tri.pts[2].y) + 
                     tri.pts[0].y*(tri.pts[2].x - tri.pts[1].x) + 
                     tri.pts[1].x*tri.pts[2].y - tri.pts[1].y*tri.pts[2].x);
  
  F32 t1 = (pt.x*(tri.pts[2].y - tri.pts[0].y) + 
            pt.y*(tri.pts[0].x - tri.pts[2].x) - 
            tri.pts[0].x*tri.pts[2].y + tri.pts[0].y*tri.pts[2].x) / denominator;
  
  F32 t2 = (pt.x*(tri.pts[1].y - tri.pts[0].y) + 
            pt.y*(tri.pts[0].x - tri.pts[1].x) - 
            tri.pts[0].x*tri.pts[1].y + tri.pts[0].y*tri.pts[1].x) / -denominator;
  
  F32 s = t1 + t2;
  
  return 0 <= t1 && t1 <= 1 && 0 <= t2 && t2 <= 1 && s <= 1;
}

static B32
_bonk_tri2_pt2_barycentric(Tri2 tri, V2 pt) {
  
  F32 denominator = ((tri.pts[1].y - tri.pts[2].y)*
                     (tri.pts[0].x - tri.pts[2].x) + (tri.pts[2].x - tri.pts[1].x)*
                     (tri.pts[0].y - tri.pts[2].y));
  
  F32 a = ((tri.pts[1].y - tri.pts[2].y)*
           (pt.x - tri.pts[2].x) + (tri.pts[2].x - tri.pts[1].x)*
           (pt.y - tri.pts[2].y)) / denominator;
  
  F32 b = ((tri.pts[2].y - tri.pts[0].y)*
           (pt.x - tri.pts[2].x) + (tri.pts[0].x - tri.pts[2].x)*
           (pt.y - tri.pts[2].y)) / denominator;
  
  F32 c = 1.f - a - b;
  
  return 0.f <= a && a <= 1.f && 0.f <= b && b <= 1.f && 0.f <= c && c <= 1.f;
  
}


static B32
_bonk_tri2_pt2_dot_product(Tri2 tri, V2 pt) {
  V2 vec0 = v2_set(pt.x - tri.pts[0].x, pt.y - tri.pts[0].y);      
  V2 vec1 = v2_set(pt.x - tri.pts[1].x, pt.y - tri.pts[1].y);      
  V2 vec2 = v2_set(pt.x - tri.pts[2].x, pt.y - tri.pts[2].y);      
  
  V2 n0 = v2_set(tri.pts[1].y - tri.pts[0].y, -tri.pts[1].x + tri.pts[0].x);
  V2 n1 = v2_set(tri.pts[2].y - tri.pts[1].y, -tri.pts[2].x + tri.pts[1].x);
  V2 n2 = v2_set(tri.pts[0].y - tri.pts[2].y, -tri.pts[0].x + tri.pts[2].x);
  
  B32 side0 = v2_dot(n0,vec0) < 0.f;
  B32 side1 = v2_dot(n1,vec1) < 0.f;
  B32 side2 = v2_dot(n2,vec2) < 0.f;
  
  return side0 == side1 && side0 == side2;
}


static B32
bonk_tri2_pt2(Tri2 tri, V2 pt) {
  return _bonk_tri2_pt2_dot_product(tri, pt);
}

static B32
bonk_circ2_circ2_set(Circ2 a, Circ2 b) {
  F32 combined_radius = a.radius + b.radius;
  return v2_dist_sq(a.center, b.center) < combined_radius*combined_radius;
}

#endif //MOMO_SHAPES_H
