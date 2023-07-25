// Authors: Gerald Wong, momodevelop
// 
//
// Notes:
// - While there could be several representation of colors,
//   I would prefer to use floating point values between [0-1] 
//   to cover all general cases. If the user needs to convert it to a more
//   usable and concrete value, say, [0 - 255], they would just need to 
//   multiply the values by 255 and go from there.
//
// Todo:
// - HSV support?
// 

#ifndef MOMO_COLORS_H
#define MOMO_COLORS_H


// Each component of rgba_t are in the range [0 - 1].
struct rgb_t {
  f32_t r, g, b;   
};

// Each component are in the range of [0 - 1].
// For hue, normally it is a number between [0 - 360], but
// it will be mapped linearly to [0 - 1] in this case.
// i.e. hue 0 is 0 degrees, hue 1 is 360 degrees.
struct hsl_t {
  f32_t h, s, l;  
};

struct hsla_t {
  union {
    struct { f32_t h,s,l; };  
    hsl_t hsl;
  };
  f32_t a;
};


struct rgba_t {
  union {
    struct { f32_t r, g, b; };  
    rgb_t rgb;
  };
  f32_t a;
};

static hsl_t  hsl_set(f32_t h, f32_t s, f32_t l);
static hsla_t hsla_set(f32_t h, f32_t s, f32_t l, f32_t a);
static rgba_t rgba_set(f32_t r, f32_t g, f32_t b, f32_t a);
static rgba_t rgba_hex(u32_t hex);  
static rgba_t hsla_to_rgba(hsla_t c);
static hsl_t  rbg_to_hsl(rgb_t c);
static rgb_t  hsl_to_rgb(hsl_t c);

#define RGBA_WHITE rgba_set(1.f, 1.f, 1.f, 1.f)
////////////////////////////////////////////////////////////
// IMPLEMENTATION
//
static rgba_t 
rgba_set(f32_t r, f32_t g, f32_t b, f32_t a){
	rgba_t ret;
	ret.r = r;
	ret.g = g;
	ret.b = b;
	ret.a = a;
	
	return ret;
}

static hsla_t 
hsla_set(f32_t h, f32_t s, f32_t l, f32_t a){
	hsla_t ret;
	ret.h = h;
	ret.s = s;
	ret.l = l;
	ret.a = a;
	
	return ret;
}

static rgba_t
rgba_hex(u32_t hex) {
  rgba_t ret;
  
  ret.r = (f32_t)((hex & 0xFF000000) >> 24)/255.f;
  ret.g = (f32_t)((hex & 0x00FF0000) >> 16)/255.f;
  ret.b = (f32_t)((hex & 0x0000FF00) >> 8)/255.f;
  ret.a = (f32_t)(hex & 0x000000FF)/255.f;
  
  return ret;
}

static hsl_t
hsl_set(f32_t h, f32_t s, f32_t l) {
  hsl_t ret;
  ret.h = h;
  ret.s = s;
  ret.l = l;
  
  return ret;     
}

static hsl_t 
rbg_to_hsl(rgb_t c) {
  assert(c.r >= 0.f &&
         c.r <= 1.f &&
         c.g >= 0.f &&
         c.g <= 1.f &&
         c.b >= 0.f &&
         c.b <= 1.f);
  hsl_t ret;
  f32_t max = max_of(max_of(c.r, c.g), c.b);
  f32_t min = min_of(min_of(c.r, c.g), c.b);
  
  f32_t delta = max - min; // aka chroma
  
  
  if (is_close_f32(max, c.r)) {
    f32_t segment = (c.g - c.b)/delta;
    f32_t shift = 0.f / 60;
    if (segment < 0) {
      shift = 360 / 60;
    }
    else {
      shift = 0.f / 60;
    }
    ret.h = (segment + shift) * 60.f;
  }
  
  else if (is_close_f32(max, c.g)) {
    f32_t segment = (c.b - c.r)/delta;
    f32_t shift = 120.f / 60.f;
    ret.h = (segment + shift) * 60.f;
  }
  
  else if (is_close_f32(max, c.b)) {
    f32_t segment = (c.r - c.g)/delta;
    f32_t shift = 240.f / 60.f;
    ret.h = ((segment + shift) * 60.f);
  }
  else {
    ret.h = 0.f;
  }
  ret.h /= 360.f;
  
  
  ret.l = (max + min) * 0.5f;
  
  if (is_close_f32(delta, 0.f)) {
    ret.s = 0.f;
  }
  else {
    ret.s = delta/(1.f - f32_abs(2.f * ret.l - 1.f));
  }
  
  return ret;
}

static f32_t 
_hue_to_color(f32_t p, f32_t q, f32_t t) {
  
  if (t < 0) 
    t += 1.f;
  if (t > 1.f) 
    t -= 1.f;
  if (t < 1./6.f) 
    return p + (q - p) * 6.f * t;
  if (t < 1./2.f) 
    return q;
  if (t < 2./3.f)   
    return p + (q - p) * (2.f/3.f - t) * 6.f;
  
  return p;
  
}



static rgb_t 
hsl_to_rgb(hsl_t c) {
  assert(c.h >= 0.f &&
         c.h <= 360.f &&
         c.s >= 0.f &&
         c.s <= 1.f &&
         c.l >= 0.f &&
         c.l <= 1.f);
  rgb_t ret;
  if(is_close_f32(c.s, 0.f)) {
    ret.r = ret.g = ret.b = c.l; // achromatic
  }
  else {
    f32_t q = c.l < 0.5f ? c.l * (1 + c.s) : c.l + c.s - c.l * c.s;
    f32_t p = 2.f * c.l - q;
    ret.r = _hue_to_color(p, q, c.h + 1.f/3.f);
    ret.g = _hue_to_color(p, q, c.h);
    ret.b = _hue_to_color(p, q, c.h - 1.f/3.f);
  }
  
  
  return ret;
  
}


static rgba_t hsla_to_rgba(hsla_t c) {
  rgba_t ret = {};
  ret.rgb = hsl_to_rgb(c.hsl);
  ret.a = c.a;

  return ret;

}

#endif //MOMO_COLORS_H

