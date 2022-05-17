/* date = April 27th 2022 8:10 pm */

#ifndef GAME_MODE_SANDBOX_H
#define GAME_MODE_SANDBOX_H

struct Edge {
  Line2 line;
};

struct Light_Triangle {
  V2 p0;
  V2 p1; 
  V2 p2;
};

struct Light {
  V2 dir;
  F32 half_angle;
  
  V2 pos;  
  U32 color;
  
  U32 intersection_count;
  V2 intersections[64];
  
  U32 debug_ray_count;
  V2 debug_rays[64];
  
  U32 triangle_count;
  Light_Triangle triangles[128];
  
};


struct Sandbox_Mode {
  V2 position;
  V2 size;
  Light* player_light;
  
  U32 endpoint_count;
  V2 endpoints[32];
  
  U32 edge_count;
  Edge edges[32];
  
  U32 light_count;
  Light lights[32];
  
};



#endif //GAME_MODE_SANDBOX_H
