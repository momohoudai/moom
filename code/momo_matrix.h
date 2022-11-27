// Authors: Gerald Wong, momodevelop
// 
// This file contains implementation of a simple linear memory allocator.
//
// Notes:
// - All matrices are in F32. I don't see any reason why they shouldn't be, 
//   at least for my use cases.
// - All matrix operations assume row-major
//
// Todo:
// - column major?
// 


#ifndef MOMO_MATRIX_H
#define MOMO_MATRIX_H

struct M44 {
	F32 e[4][4];
};

static M44 m44_concat(M44 lhs, M44 rhs);
static M44 m44_transpose(M44 m);
static M44 m44_scale(F32 x, F32 y, F32 z);
static M44 m44_identity();
static M44 m44_translation(F32 x, F32 y, F32 z);
static M44 m44_rotation_x(F32 radians);
static M44 m44_rotation_y(F32 radians);
static M44 m44_rotation_z(F32 radians);
static M44 m44_orthographic(F32 left, F32 right, F32 bottom, F32 top, F32 near, F32 far);
static M44 m44_frustum(F32 left, F32 right, F32 bottom, F32 top, F32 near, F32 far);
static M44 m44_perspective(F32 fov, F32 aspect, F32 near, F32 far);

#if IS_CPP
static M44 operator*(M44 lhs, M44 rhs);
#endif // IS_CPP

//////////////////////////////////////////////////////////////////
// IMPLEMENTATION

static M44
m44_concat(M44 lhs, M44 rhs) {
	M44 ret = {};
  for (U32 r = 0; r < 4; r++) { 
    for (U32 c = 0; c < 4; c++) { 
      for (U32 i = 0; i < 4; i++) {
				ret.e[r][c] += lhs.e[r][i] *  rhs.e[i][c]; 
			}
		} 
	} 
	return ret;
}

static M44 
m44_transpose(M44 m) {
	M44 ret = {};
	for (U32 i = 0; i < 4; ++i ) {
		for (U32 j = 0; j < 4; ++j) {
			ret.e[i][j] = m.e[j][i];
		}
	}
	return ret;
}
static M44 m44_scale(F32 x, F32 y, F32 z = 1.f) {
	M44 ret = {};
	ret.e[0][0] = x;
	ret.e[1][1] = y;
	ret.e[2][2] = z;
	ret.e[3][3] = 1.f;
	
	return ret;
}

static M44 
m44_identity() {
	M44 ret = {};
	ret.e[0][0] = 1.f;
	ret.e[1][1] = 1.f;
	ret.e[2][2] = 1.f;
	ret.e[3][3] = 1.f;
	
	return ret;
}

static M44 
m44_translation(F32 x, F32 y, F32 z = 0.f) {
	M44 ret = m44_identity();
	ret.e[0][3] = x;
	ret.e[1][3] = y;
	ret.e[2][3] = z;
	ret.e[3][3] = 1.f;
	
	return ret;
}

static M44 
m44_rotation_x(F32 rad) {
	// NOTE(Momo): 
	// 1  0  0  0
	// 0  c -s  0
	// 0  s  c  0
	// 0  0  0  1
	F32 c = cos_f32(rad);
	F32 s = sin_f32(rad);
	M44 ret = {};
	ret.e[0][0] = 1.f;
	ret.e[3][3] = 1.f;
	ret.e[1][1] = c;
	ret.e[1][2] = -s;
	ret.e[2][1] = s;
	ret.e[2][2] = c;
	
	return ret;
}
static M44 m44_rotation_y(F32 rad) {
	
	// NOTE(Momo): 
	//  c  0  s  0
	//  0  1  0  0
	// -s  0  c  0
	//  0  0  0  1
	F32 c = cos_f32(rad);
	F32 s = sin_f32(rad);
	M44 ret = {};
	ret.e[0][0] = c;
	ret.e[0][2] = s;
	ret.e[1][1] = 1.f;
	ret.e[2][0] = -s;
	ret.e[2][2] = c;
	ret.e[3][3] = 1.f;
	
	return ret;
}

static M44 
m44_rotation_z(F32 rad) {
	// NOTE(Momo): 
	//  c -s  0  0
	//  s  c  0  0
	//  0  0  1  0
	//  0  0  0  1
	
	F32 c = cos_f32(rad);
	F32 s = sin_f32(rad);
	M44 ret = {};
	ret.e[0][0] = c;
	ret.e[0][1] = -s;
	ret.e[1][0] = s;
	ret.e[1][1] = c;
	ret.e[2][2] = 1.f;
	ret.e[3][3] = 1.f;
	
	return ret;
}

static M44 
m44_orthographic(F32 left, F32 right, F32 bottom, F32 top, F32 near, F32 far) {
	
	M44 ret = {0};
	ret.e[0][0] = 2.f/(right-left);
	ret.e[1][1] = 2.f/(top-bottom);
	ret.e[2][2] = 2.f/(far-near);
	ret.e[3][3] = 1.f;
	ret.e[0][3] = -(right+left)/(right-left);
	ret.e[1][3] = -(top+bottom)/(top-bottom);
	ret.e[2][3] = -(far+near)/(far-near);
	
	return ret;
}

static M44 
m44_frustum(F32 left, F32 right, F32 bottom, F32 top, F32 near, F32 far) {
	M44 ret = {};
	ret.e[0][0] = (2.f*near)/(right-left);
	ret.e[1][1] = (2.f*near)/(top-bottom);
	ret.e[2][2] = -(far+near)/(far-near);
	ret.e[3][2] = 1;  
	ret.e[0][2] = (right+left)/(right-left);
	ret.e[1][2] = (top+bottom)/(top-bottom);
	ret.e[1][3] = -near*(top+bottom)/(top-bottom);
	ret.e[2][3] = 2.f*far*near/(far-near);
	
	return ret;
}

static M44 
m44_perspective(F32 fov, F32 aspect, F32 near, F32 far){
	F32 top = near * tan_f32(fov*0.5f);
	F32 right = top * aspect;
	return m44_frustum(-right, right,
                     -top, top,
                     near, far);
}

#if IS_CPP
static M44 operator*(M44 lhs, M44 rhs) {
  return m44_concat(lhs, rhs);
}
#endif // IS_CPP


#endif //MOMO_MATRIX_H
