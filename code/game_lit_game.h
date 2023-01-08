
struct lit_edge_t {
  b32_t is_disabled;
  v2f_t start_pt;
  v2f_t end_pt;
};



struct lit_light_intersection_t {
  b32_t is_shell;
  v2f_t pt;
};

struct lit_light_triangle_t {
  v2f_t p0, p1, p2;
};

struct lit_light_t {
  v2f_t dir;
  f32_t half_angle;
  
  v2f_t pos;  
  u32_t color;

  u32_t triangle_count;
  lit_light_triangle_t triangles[256];

  u32_t intersection_count;
  lit_light_intersection_t intersections[256];

};



enum lit_light_type_t {
  Lit_LIGHT_TYPE_POINT,
  Lit_LIGHT_TYPE_DIRECTIONAL,
  Lit_LIGHT_TYPE_WEIRD
};
\



struct lit_particle_t {
  v2f_t pos, vel;
  v2f_t size_start, size_end;
  rgba_t color_start, color_end;
  asset_sprite_id_t sprite_id;
  f32_t lifespan;
  f32_t lifespan_now;
};

struct lit_particle_pool_t {
  f32_t particle_lifespan_min;
  f32_t particle_lifespan_max;

  f32_t particle_color_start;
  f32_t particle_color_end;

  u32_t particle_count;
  lit_particle_t particles[256];
};




struct lit_sensor_t {
  v2f_t pos;
  u32_t target_color;
  u32_t current_color;
  f32_t particle_cd;
};



struct lit_player_t {
  v2f_t pos;
  lit_light_t* held_light;
  b32_t is_holding_light;

  // For animating getting the light
  f32_t light_retrival_time;
  v2f_t old_light_pos;
};


