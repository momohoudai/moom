#ifndef MOMO_SHAPES_H
#define MOMO_SHAPES_H


typedef struct Rect2 {
  V2 min, max;
}Rect2;





///////////////////////////////////////////////////////////////////
// IMPLEMENTATION



// https://totologic.blogspot.com/2014/01/accurate-point-in-triangle-test.html
// NOTE(Momo): We should really profile to see which is the best but I'm assuming
// it's the dot product one
static B32
_bonk_tri2_pt2_parametric(V2 tp0, V2 tp1, V2 tp2, V2 pt) {
  F32 denominator = (tp0.x*(tp1.y - tp2.y) + 
                     tp0.y*(tp2.x - tp1.x) + 
                     tp1.x*tp2.y - tp1.y*tp2.x);
  
  F32 t1 = (pt.x*(tp2.y - tp0.y) + 
            pt.y*(tp0.x - tp2.x) - 
            tp0.x*tp2.y + tp0.y*tp2.x) / denominator;
  
  F32 t2 = (pt.x*(tp1.y - tp0.y) + 
            pt.y*(tp0.x - tp1.x) - 
            tp0.x*tp1.y + tp0.y*tp1.x) / -denominator;
  
  F32 s = t1 + t2;
  
  return 0 <= t1 && t1 <= 1 && 0 <= t2 && t2 <= 1 && s <= 1;
}

static B32
_bonk_tri2_pt2_barycentric(V2 tp0, V2 tp1, V2 tp2, V2 pt) {
  
  F32 denominator = ((tp1.y - tp2.y)*
                     (tp0.x - tp2.x) + (tp2.x - tp1.x)*
                     (tp0.y - tp2.y));
  
  F32 a = ((tp1.y - tp2.y)*
           (pt.x - tp2.x) + (tp2.x - tp1.x)*
           (pt.y - tp2.y)) / denominator;
  
  F32 b = ((tp2.y - tp0.y)*
           (pt.x - tp2.x) + (tp0.x - tp2.x)*
           (pt.y - tp2.y)) / denominator;
  
  F32 c = 1.f - a - b;
  
  return 0.f <= a && a <= 1.f && 0.f <= b && b <= 1.f && 0.f <= c && c <= 1.f;
  
}


static B32
_bonk_tri2_pt2_dot_product(V2 tp0, V2 tp1, V2 tp2, V2 pt) {
  V2 vec0 = v2_set(pt.x - tp0.x, pt.y - tp0.y);      
  V2 vec1 = v2_set(pt.x - tp1.x, pt.y - tp1.y);      
  V2 vec2 = v2_set(pt.x - tp2.x, pt.y - tp2.y);      
  
  V2 n0 = v2_set(tp1.y - tp0.y, -tp1.x + tp0.x);
  V2 n1 = v2_set(tp2.y - tp1.y, -tp2.x + tp1.x);
  V2 n2 = v2_set(tp0.y - tp2.y, -tp0.x + tp2.x);
  
  B32 side0 = v2_dot(n0,vec0) < 0.f;
  B32 side1 = v2_dot(n1,vec1) < 0.f;
  B32 side2 = v2_dot(n2,vec2) < 0.f;
  
  return side0 == side1 && side0 == side2;
}


static B32
bonk_tri2_pt2(V2 tp0, V2 tp1, V2 tp2, V2 pt) {
  return _bonk_tri2_pt2_dot_product(tp0, tp1, tp2, pt);
}



#endif //MOMO_SHAPES_H
