


//
// Points
//
static v2f_t*
lit_game_push_point(lit_game_t* m, v2f_t point) {
  assert(m->point_count < array_count(m->points));
  m->points[m->point_count] = point;
  return m->points + m->point_count++;
}

// 
// Animator
//


static void 
lit_game_animator_push_patrol_point_waypoint(
    lit_game_animator_t* animator, 
    f32_t pos_x, 
    f32_t pos_y)
{
  assert(animator->type == LIT_ANIMATOR_TYPE_PATROL_POINT);
  auto* a = &animator->move_point;

  // Add the waypoint
  assert(a->waypoint_count < array_count(a->waypoints));
  v2f_t* wp = a->waypoints + a->waypoint_count++;
  wp->x = pos_x;
  wp->y = pos_y;


}

static lit_game_animator_t*
lit_game_animator_begin_patrol_point(lit_game_t* m, f32_t duration_per_waypoint, v2f_t* point) 
{
  assert(m->animator_count < array_count(m->animators));
  auto* anim = m->animators + m->animator_count++;
  anim->type = LIT_ANIMATOR_TYPE_PATROL_POINT;

  auto* a = &anim->move_point;
  a->timer = 0.f;
  a->duration = duration_per_waypoint;
  a->point = point;
  a->waypoint_count = 0;
  a->current_waypoint_index = 0;

  lit_game_animator_push_patrol_point_waypoint(anim, point->x, point->y);

  return anim;
}

static void 
lit_game_animator_end_patrol_point(lit_game_animator_t* anim)
{
  auto* a = &anim->move_point;
  a->timer = 0.f;
  a->current_waypoint_index = 0;
  u32_t next_waypoint_index = (a->current_waypoint_index + 1) % a->waypoint_count;
  a->start = a->waypoints[a->current_waypoint_index];
  a->end = a->waypoints[next_waypoint_index];
}


static lit_game_animator_rotate_point_t* 
lit_game_animator_push_rotate_point(
    lit_game_t* m,
    v2f_t* pt_to_rotate,
    v2f_t* pt_of_rotation,
    f32_t speed) 
{

  assert(m->animator_count < array_count(m->animators));
  auto* anim = m->animators + m->animator_count++;
  anim->type = LIT_ANIMATOR_TYPE_ROTATE_POINT;

  auto* a = &anim->rotate_point;
  a->speed = speed;
  a->point_of_rotation = pt_of_rotation;
  a->point = pt_to_rotate;
  a->delta = dref(pt_to_rotate) - dref(pt_of_rotation);

  return a;
}

static void 
lit_game_animate(lit_game_animator_t* animator, f32_t dt) {
  switch(animator->type) {
    case LIT_ANIMATOR_TYPE_PATROL_POINT:{
      auto* a = &animator->move_point;
      a->timer += dt;
      if (a->timer > a->duration) {
        a->timer = 0.f;
        a->current_waypoint_index = ++a->current_waypoint_index % a->waypoint_count;
        u32_t next_waypoint_index = (a->current_waypoint_index + 1) % a->waypoint_count;
        a->start = a->waypoints[a->current_waypoint_index];
        a->end = a->waypoints[next_waypoint_index];
      }

      f32_t alpha = f32_ease_inout_sine(a->timer/a->duration);
      *a->point = v2f_lerp(a->start, a->end, alpha);
    } break;
    case LIT_ANIMATOR_TYPE_ROTATE_POINT: {
      auto* a = &animator->rotate_point;
      a->delta = v2f_rotate(a->delta, a->speed * dt);
      dref(a->point) = dref(a->point_of_rotation) + a->delta;
    } break;

  }
}

static void
lit_game_animate_everything(lit_game_t* m, f32_t dt) {
  for(u32_t animator_index = 0; animator_index < m->animator_count; ++animator_index)
  {
    lit_game_animate(m->animators + animator_index, dt);
  }
}

//
// Edges
//

static void 
lit_game_calc_ghost_edge_line(lit_game_edge_t* e, v2f_t* min, v2f_t* max) {
  v2f_t dir = v2f_norm(e->end_pt - e->start_pt) * 0.0001f;

  *min = v2f_sub(e->start_pt, dir);
  *max = v2f_add(e->end_pt, dir);
}

static lit_game_edge_t*
lit_game_push_edge(lit_game_t* m, f32_t min_x, f32_t min_y, f32_t max_x, f32_t max_y) {
  assert(m->edge_count < array_count(m->edges));
  lit_game_edge_t* edge = m->edges + m->edge_count++;

  edge->start_pt = v2f_set(min_x, min_y);
  edge->end_pt = v2f_set(max_x, max_y);

  // TODO: can we remove this part?
  v2f_t p0, p1;
  lit_game_calc_ghost_edge_line(edge, &p0, &p1);
  edge->start_pt = p0;
  edge->end_pt = p1;

  return edge;
}




static void 
lit_game_push_box(lit_game_t* m, f32_t min_x, f32_t min_y, f32_t max_x, f32_t max_y) 
{
  lit_game_push_edge(m, min_x, min_y, max_x, min_y);
  lit_game_push_edge(m, max_x, min_y, max_x, max_y);
  lit_game_push_edge(m, max_x, max_y, min_x, max_y);
  lit_game_push_edge(m, min_x, max_y, min_x, min_y);
}

static void 
lit_game_push_aabb(lit_game_t* m, f32_t cx, f32_t cy, f32_t hw, f32_t hh) {
  f32_t min_x = cx - hw;
  f32_t max_x = cx + hw;
  f32_t min_y = cy - hh;
  f32_t max_y = cy + hh;

  lit_game_push_edge(m, min_x, min_y, max_x, min_y);
  lit_game_push_edge(m, max_x, min_y, max_x, max_y);
  lit_game_push_edge(m, max_x, max_y, min_x, max_y);
  lit_game_push_edge(m, min_x, max_y, min_x, min_y);
}

static lit_game_double_edge_t 
lit_game_push_double_edge(lit_game_t* m, f32_t min_x, f32_t min_y, f32_t max_x, f32_t max_y) {
  auto* e1 = lit_game_push_edge(m, min_x, min_y, max_x, max_y);
  auto* e2 = lit_game_push_edge(m, max_x, max_y, min_x, min_y);
  return { e1, e2 };
}

static void
lit_game_push_rotating_double_edge(
    lit_game_t* g, 
    f32_t min_x, f32_t min_y, f32_t max_x, f32_t max_y,
    v2f_t* pt_of_rotation_for_min,
    v2f_t* pt_of_rotation_for_max,
    f32_t speed_for_min,
    f32_t speed_for_max)
{
  auto edges = lit_game_push_double_edge(g, min_x, min_y, max_x, max_y);

  if (pt_of_rotation_for_min) {
    lit_game_animator_push_rotate_point(g, &edges.e1->start_pt, pt_of_rotation_for_min, speed_for_min);
    lit_game_animator_push_rotate_point(g, &edges.e2->end_pt, pt_of_rotation_for_min, speed_for_min);
  }

  if (pt_of_rotation_for_max) {
    lit_game_animator_push_rotate_point(g, &edges.e1->end_pt, pt_of_rotation_for_max, speed_for_max);
    lit_game_animator_push_rotate_point(g, &edges.e2->start_pt, pt_of_rotation_for_max, speed_for_max);
  }
}

static void
lit_game_begin_patrolling_double_edge(lit_game_t* g, f32_t min_x, f32_t min_y, f32_t max_x, f32_t max_y, f32_t duration_per_waypoint)
{
  assert(!g->selected_animator_for_double_edge_min[0]);
  assert(!g->selected_animator_for_double_edge_min[1]);
  assert(!g->selected_animator_for_double_edge_max[0]);
  assert(!g->selected_animator_for_double_edge_max[1]);

  //
  // NOTE(momo): this is kind of disgusting because it is possible that not all points need to be patrolling,
  // so we are potentially wasting space, but whatevers....
  //
  auto edges = lit_game_push_double_edge(g, min_x, min_y, max_x, max_y);
  g->selected_animator_for_double_edge_min[0] = 
    lit_game_animator_begin_patrol_point(g, duration_per_waypoint, &edges.e1->start_pt);
  g->selected_animator_for_double_edge_min[1] = 
    lit_game_animator_begin_patrol_point(g, duration_per_waypoint, &edges.e2->end_pt);
  g->selected_animator_for_double_edge_max[0] = 
    lit_game_animator_begin_patrol_point(g, duration_per_waypoint, &edges.e1->end_pt);
  g->selected_animator_for_double_edge_max[1] = 
    lit_game_animator_begin_patrol_point(g, duration_per_waypoint, &edges.e2->start_pt);
}

static void
lit_game_push_patrolling_double_edge_waypoint_for_min(lit_game_t* g, f32_t x, f32_t y){

  assert(g->selected_animator_for_double_edge_min[0]);
  assert(g->selected_animator_for_double_edge_min[1]);

  lit_game_animator_push_patrol_point_waypoint(g->selected_animator_for_double_edge_min[0], x, y);
  lit_game_animator_push_patrol_point_waypoint(g->selected_animator_for_double_edge_min[1], x, y);
}
static void
lit_game_push_patrolling_double_edge_waypoint_for_max(lit_game_t* g, f32_t x, f32_t y){

  assert(g->selected_animator_for_double_edge_max[0]);
  assert(g->selected_animator_for_double_edge_max[1]);

  lit_game_animator_push_patrol_point_waypoint(g->selected_animator_for_double_edge_max[0], x, y);
  lit_game_animator_push_patrol_point_waypoint(g->selected_animator_for_double_edge_max[1], x, y);
}

static void
lit_game_end_patrolling_double_edge(lit_game_t* g) {

  assert(g->selected_animator_for_double_edge_min[0]);
  assert(g->selected_animator_for_double_edge_min[1]);
  assert(g->selected_animator_for_double_edge_max[0]);
  assert(g->selected_animator_for_double_edge_max[1]);

  lit_game_animator_end_patrol_point(g->selected_animator_for_double_edge_min[0]);
  lit_game_animator_end_patrol_point(g->selected_animator_for_double_edge_min[1]);
  lit_game_animator_end_patrol_point(g->selected_animator_for_double_edge_max[0]);
  lit_game_animator_end_patrol_point(g->selected_animator_for_double_edge_max[1]);

  g->selected_animator_for_double_edge_min[0] = nullptr; 
  g->selected_animator_for_double_edge_min[1] = nullptr;
  g->selected_animator_for_double_edge_max[0] = nullptr; 
  g->selected_animator_for_double_edge_max[1] = nullptr;
}


//
// LIGHT
//
static lit_game_light_t*
lit_game_push_light(lit_game_t* m, f32_t pos_x, f32_t pos_y, u32_t color, f32_t angle, f32_t turn) {
  assert(m->light_count < array_count(m->lights));
  lit_game_light_t* light = m->lights + m->light_count++;
  light->pos.x = pos_x;
  light->pos.y = pos_y;
  light->color = color;

  f32_t rad = f32_turns_to_radians(turn);
  light->dir.x = f32_cos(rad);
  light->dir.y = f32_sin(rad);
  light->half_angle = f32_deg_to_rad(angle/2.f);

  return light;
}

// Returns F32_INFINITY() if cannot find
static f32_t
lit_game_get_ray_intersection_time_wrt_edges(
    v2f_t ray_origin, 
    v2f_t ray_dir,
    lit_game_edge_t* edges,
    u32_t edge_count,
    b32_t clamp_to_ray_max = false)
{
  f32_t lowest_t1 = clamp_to_ray_max ? 1.f : F32_INFINITY;

  for(u32_t edge_index = 0; edge_index < edge_count; ++edge_index)
  {
    lit_game_edge_t* edge = edges + edge_index;


    //if (edge->is_disabled) continue;

    v2f_t edge_ray_origin = edge->start_pt;
    v2f_t edge_ray_dir = edge->end_pt - edge->start_pt;
    
    // Check for parallel
    v2f_t ray_normal = {};
    ray_normal.x = ray_dir.y;
    ray_normal.y = -ray_dir.x;


    if (!is_close_f32(v2f_dot(ray_normal, edge_ray_dir), 0.f)) {
      f32_t t2 = 
        (ray_dir.x*(edge_ray_origin.y - ray_origin.y) + 
         ray_dir.y*(ray_origin.x - edge_ray_origin.x))/
        (edge_ray_dir.x*ray_dir.y - edge_ray_dir.y*ray_dir.x);

      f32_t t1 = (edge_ray_origin.x + edge_ray_dir.x * t2 - ray_origin.x)/ray_dir.x;

      if (0.f < t1 && 
          0.f < t2 && 
          t2 < 1.f)
      {

        if (t1 < lowest_t1) {
          lowest_t1 = t1;
        }
      }
    }
  }
  return lowest_t1;
}
static void
lit_game_push_triangle(lit_game_light_t* l, v2f_t p0, v2f_t p1, v2f_t p2, u32_t color) {
  assert(l->triangle_count < array_count(l->triangles));
  lit_game_light_triangle_t* tri = l->triangles + l->triangle_count++;
  tri->p0 = p0;
  tri->p1 = p1;
  tri->p2 = p2;
}


static void
lit_gen_light_intersections(lit_game_light_t* l,
    lit_game_edge_t* edges,
    u32_t edge_count,
    arena_t* tmp_arena)
{
  //moe_profile_block(light_generation);
  arena_set_revert_point(tmp_arena);

  lit_game_light_type_t light_type = Lit_LIGHT_TYPE_POINT;
  if (l->half_angle < PI_32/2) {
    light_type = Lit_LIGHT_TYPE_DIRECTIONAL; 
  }
  else if(l->half_angle < PI_32) {
    light_type = Lit_LIGHT_TYPE_WEIRD;
  }

  l->intersection_count = 0;
  l->triangle_count = 0;


  
  f32_t offset_angles[] = {0.0f, 0.001f, -0.001f};
  //f32_t offset_angles[] = {0.0f};
  for (u32_t offset_index = 0;
      offset_index < array_count(offset_angles);
      ++offset_index) 
  {
    f32_t offset_angle = offset_angles[offset_index];
    // For each endpoint
    for(u32_t edge_index = 0; edge_index < edge_count; ++edge_index) 
    {
      lit_game_edge_t* edge = edges + edge_index;

      v2f_t ep = edge->end_pt;      

      // ignore endpoints that are not within the angle 
      f32_t angle = v2f_angle(l->dir, ep - l->pos);
      if (light_type == Lit_LIGHT_TYPE_WEIRD || 
          light_type == Lit_LIGHT_TYPE_DIRECTIONAL) 
      {
        if (angle > l->half_angle) continue;
      }
      else // light_type == Lit_LIGHT_TYPE_POINT 
      {
        // if it's a point light, we don't do anything here.
      }


      v2f_t light_ray_dir = v2f_rotate(ep - l->pos, offset_angle);

      f32_t t = lit_game_get_ray_intersection_time_wrt_edges(l->pos, light_ray_dir, edges, edge_count, offset_index == 0);

      assert(l->intersection_count < array_count(l->intersections));
      lit_light_intersection_t* intersection = l->intersections + l->intersection_count++;
      intersection->pt = (t == F32_INFINITY) ? ep : l->pos + t*light_ray_dir;
      intersection->is_shell = false;
    }
  }

  // Consider 'shell rays', which are rays that are at the 
  // extreme ends of the light (only for non-point lights)
  if (light_type != Lit_LIGHT_TYPE_POINT)
  {
    for (u32_t offset_index = 0;
        offset_index < array_count(offset_angles);
        ++offset_index) 
    { 
      f32_t offset_angle = offset_angles[offset_index];

      v2f_t dirs[2]; 
      dirs[0] = v2f_rotate(l->dir, l->half_angle + offset_angle);
      dirs[1] = v2f_rotate(l->dir, -l->half_angle + offset_angle);
      for (u32_t i = 0; i < 2; ++i) {
        f32_t t = lit_game_get_ray_intersection_time_wrt_edges(l->pos, dirs[i], edges, edge_count);

        assert(l->intersection_count < array_count(l->intersections));
        lit_light_intersection_t* intersection = l->intersections + l->intersection_count++;
        intersection->pt = l->pos + t*dirs[i];
        intersection->is_shell = true;
      }
    }
  }

  if (l->intersection_count > 0) {
    sort_entry_t* sorted_its = arena_push_arr(sort_entry_t, tmp_arena, l->intersection_count);
    assert(sorted_its);
    for (u32_t its_id = 0; 
        its_id < l->intersection_count; 
        ++its_id) 
    {
      lit_light_intersection_t* its = l->intersections + its_id;
      v2f_t basis_vec = v2f_t{1.f, 0.f};
      v2f_t intersection_vec = its->pt - l->pos;
      f32_t key = v2f_angle(basis_vec, intersection_vec);
      if (intersection_vec.y < 0.f) 
        key = PI_32*2.f - key;

      sorted_its[its_id].index = its_id;
      sorted_its[its_id].key = key; 
    }

    quicksort(sorted_its, l->intersection_count);

    for (u32_t sorted_its_id = 0;
        sorted_its_id < l->intersection_count - 1;
        sorted_its_id++)
    {
      lit_light_intersection_t* its0 = l->intersections + sorted_its[sorted_its_id].index;
      lit_light_intersection_t* its1 = l->intersections + sorted_its[sorted_its_id+1].index;

      b32_t ignore = false;

      // In the case of 'wierd' lights,
      // shell ray should not have a triangle to another shell ray 
      if (light_type == Lit_LIGHT_TYPE_WEIRD) {
        if (its0->is_shell && its1->is_shell) {
          ignore = true;
        }
      }

      if (!ignore) {
        v2f_t p0 = its0->pt;
        v2f_t p1 = l->pos;
        v2f_t p2 = its1->pt;

        // Make sure we are going CCW
        if (v2f_cross(p0-p1, p2-p1) > 0.f) {
          lit_game_push_triangle(l, p0, p1, p2, l->color);
        }
      }
    }

    lit_light_intersection_t* its0 = l->intersections + sorted_its[l->intersection_count-1].index;
    lit_light_intersection_t* its1 = l->intersections + sorted_its[0].index;

    // In the case of 'wierd' lights,
    // shell ray should not have a triangle to another shell ray 

    b32_t ignore = false;
    if (light_type == Lit_LIGHT_TYPE_WEIRD) {
      if (its0->is_shell && its1->is_shell) {
        ignore = true;
      }
    }

    if (!ignore) {
      v2f_t p0 = its0->pt;
      v2f_t p1 = l->pos;
      v2f_t p2 = its1->pt;

      // Make sure we are going CCW
      if (v2f_cross(p0-p1, p2-p1) > 0.f) {
        lit_game_push_triangle(l, p0, p1, p2, l->color);
      }
    }
  }

}


static void
lit_gen_lights(
    lit_game_light_t* lights, 
    u32_t light_count,
    lit_game_edge_t* edges,
    u32_t edge_count,
    arena_t* tmp_arena)  {
  // Update all lights
  for(u32_t light_index = 0; light_index < light_count; ++light_index)
  {
    lit_game_light_t* light = lights + light_index;
    lit_gen_light_intersections(light, edges, edge_count, tmp_arena);
  }

}

static void
lit_game_render_lights(lit_game_t* g) {
  //
  // Emitters
  //
  for(u32_t light_index = 0; light_index < g->light_count; ++light_index)
  {
    lit_game_light_t* light = g->lights + light_index;
    gfx_push_asset_sprite(gfx, &lit->assets, 
        g->filled_circle_sprite, 
        light->pos,
        v2f_set(16.f, 16.f),
        rgba_set(0.8f, 0.8f, 0.8f, 1.f));
    gfx_advance_depth(gfx);
  }

  //
  // Lights
  //
  gfx_set_blend_additive(gfx);
  for(u32_t light_index = 0; light_index < g->light_count; ++light_index)
  {
    lit_game_light_t* l = g->lights + light_index;
    for(u32_t tri_index = 0; tri_index < l->triangle_count; ++tri_index)
    {
      lit_game_light_triangle_t* lt = l->triangles + tri_index;
      gfx_draw_filled_triangle(gfx, 
          rgba_hex(l->color),
          lt->p0,
          lt->p1,
          lt->p2);
    } 
    gfx_advance_depth(gfx);
  }
  gfx_set_blend_alpha(gfx);
}



static void
lit_draw_light_rays(lit_game_t* g) {

  for(u32_t light_index = 0; light_index < g->light_count; ++light_index) 
  {
    lit_game_light_t* light = g->lights + light_index;
    for_cnt(intersection_index, light->intersection_count){
      lit_light_intersection_t* intersection = light->intersections + intersection_index;
      gfx_draw_line(gfx, light->pos, intersection->pt, 2.f, rgba_hex(0xFF0000FF));

    }

  }
  gfx_advance_depth(gfx);
}

static void
lit_draw_edges(lit_game_t* g) {

  for(u32_t edge_index = 0; edge_index < g->edge_count; ++edge_index) 
  {
    lit_game_edge_t* edge = g->edges + edge_index;
    gfx_draw_line(gfx, edge->start_pt, edge->end_pt, 3.f, rgba_hex(0x888888FF));
  }
  gfx_advance_depth(gfx);
}

//
// Player
//
static void
lit_init_player(lit_game_t* g, f32_t x, f32_t y) {
  lit_game_player_t* player = &g->player; 
  player->held_light = nullptr;
  player->pos.x = x;
  player->pos.y = y;
  player->light_hold_mode = LIT_PLAYER_LIGHT_HOLD_MODE_NONE;
}

static void
lit_game_player_release_light(lit_game_t* g) {
  lit_game_player_t* player = &g->player;
  player->held_light = nullptr;
  player->light_hold_mode = LIT_PLAYER_LIGHT_HOLD_MODE_NONE;
  pf->show_cursor();
}

static void
lit_game_player_find_nearest_light(lit_game_t* g) {
  lit_game_player_t* player = &g->player;
  f32_t shortest_dist = LIT_PLAYER_PICKUP_DIST; // limit
  player->nearest_light = nullptr;

  for(u32_t light_index = 0; light_index < g->light_count; ++light_index) {
    lit_game_light_t* l = g->lights +light_index;
    f32_t dist = v2f_dist_sq(l->pos, player->pos);
    if (shortest_dist > dist) {
      player->nearest_light = l;
      shortest_dist = dist;
    }
  }
}


static void
lit_game_player_hold_nearest_light_if_empty_handed(
    lit_game_t* g, 
    lit_game_player_light_hold_mode_t light_hold_mode) 
{
  lit_game_player_t* player = &g->player;

  if (player->light_hold_mode == LIT_PLAYER_LIGHT_HOLD_MODE_NONE) {
    if (player->nearest_light) {          
      player->held_light = player->nearest_light;
      player->old_light_pos = player->nearest_light->pos;
      player->light_retrival_time = 0.f;
      player->light_hold_mode = light_hold_mode;
      pf->hide_cursor();
    }
  }
}

static void 
lit_game_update_player(lit_game_t* g, f32_t dt) 
{
  lit_game_player_t* player = &g->player; 

  player->pos.x = input->mouse_pos.x;
  player->pos.y = LIT_HEIGHT - input->mouse_pos.y;

  lit_game_player_find_nearest_light(g);

  //
  // Move light logic
  //
  if (input_is_button_poked(input->buttons[INPUT_BUTTON_CODE_LMB])) {
    lit_game_player_hold_nearest_light_if_empty_handed(g, LIT_PLAYER_LIGHT_HOLD_MODE_MOVE);
  }

  else if (input_is_button_released(input->buttons[INPUT_BUTTON_CODE_LMB]))
  {
    lit_game_player_release_light(g);
  }

  //
  // Rotate light logic
  //
  if (input_is_button_poked(input->buttons[INPUT_BUTTON_CODE_RMB]))
  {
    lit_game_player_hold_nearest_light_if_empty_handed(g, LIT_PLAYER_LIGHT_HOLD_MODE_ROTATE);
    pf->lock_cursor();
    player->locked_pos_x = player->pos.x;

  }
  else if (input_is_button_released(input->buttons[INPUT_BUTTON_CODE_RMB])) 
  {
    lit_game_player_release_light(g);
    pf->unlock_cursor();
  }

  // Restrict movement
  if (player->pos.x > LIT_WIDTH - LIT_PLAYER_RADIUS) {
    player->pos.x = LIT_WIDTH - LIT_PLAYER_RADIUS;
  }
  if (player->pos.x < LIT_PLAYER_RADIUS) {
    player->pos.x = LIT_PLAYER_RADIUS;
  }
  if (player->pos.y > LIT_HEIGHT - LIT_PLAYER_RADIUS) {
    player->pos.y = LIT_HEIGHT - LIT_PLAYER_RADIUS;
  }
  if (player->pos.y < LIT_PLAYER_RADIUS) {
    player->pos.y = LIT_PLAYER_RADIUS;
  }

  //
  // Actual player control logic
  //
  switch(player->light_hold_mode) {
    case LIT_PLAYER_LIGHT_HOLD_MODE_NONE: {
      // do nothing
    } break;
    case LIT_PLAYER_LIGHT_HOLD_MODE_MOVE: {
      // Move the held light to player's position
      if (player->held_light) {
        if (player->light_retrival_time < LIT_PLAYER_LIGHT_RETRIEVE_DURATION) {
          player->light_retrival_time += dt;
        }
        else {
          player->light_retrival_time = LIT_PLAYER_LIGHT_RETRIEVE_DURATION;
        }
        f32_t ratio = player->light_retrival_time / LIT_PLAYER_LIGHT_RETRIEVE_DURATION; 
        player->held_light->pos.x = f32_lerp(player->old_light_pos.x, player->pos.x, ratio) ;
        player->held_light->pos.y = f32_lerp(player->old_light_pos.y, player->pos.y,  ratio) ;
      }
    } break;
    case LIT_PLAYER_LIGHT_HOLD_MODE_ROTATE: {
      if (player->held_light != nullptr) {
        f32_t mouse_delta = player->pos.x - player->locked_pos_x;
        player->held_light->dir = 
          v2f_rotate(player->held_light->dir, LIT_PLAYER_ROTATE_SPEED * dt * mouse_delta );
      }

    } break;
  }
}

static void
lit_game_render_player(lit_game_t* g){
  lit_game_player_t* player = &g->player;


  if (player->light_hold_mode == LIT_PLAYER_LIGHT_HOLD_MODE_NONE) { 
    if (player->nearest_light) {
      gfx_push_asset_sprite(gfx, &lit->assets,
          g->circle_sprite, 
          player->nearest_light->pos, 
          v2f_set(LIT_PLAYER_RADIUS*2, LIT_PLAYER_RADIUS*2));
      gfx_advance_depth(gfx);
    }
  }
  else if (player->light_hold_mode == LIT_PLAYER_LIGHT_HOLD_MODE_ROTATE) {
    gfx_push_asset_sprite(gfx, &lit->assets,
        g->rotate_sprite, 
        player->held_light->pos, 
        v2f_set(LIT_PLAYER_RADIUS*2, LIT_PLAYER_RADIUS*2));
    gfx_advance_depth(gfx);

  }
  else if (player->light_hold_mode == LIT_PLAYER_LIGHT_HOLD_MODE_MOVE) {
    gfx_push_asset_sprite(gfx, &lit->assets,
        g->move_sprite, 
        player->held_light->pos, 
        v2f_set(LIT_PLAYER_RADIUS*2, LIT_PLAYER_RADIUS*2));
    gfx_advance_depth(gfx);


  }
}


//
// Particles
//
static void
lit_spawn_particle(
    lit_game_t* g,
    f32_t lifespan,
    v2f_t pos, v2f_t vel,
    rgba_t color_start,
    rgba_t color_end,
    v2f_t size_start,
    v2f_t size_end) 
{
  lit_particle_pool_t* ps = &g->particles;
  if (ps->particle_count < array_count(ps->particles)) {
    lit_particle_t* p = ps->particles + ps->particle_count++; 
    p->pos = pos;
    p->vel = vel;
    p->color_start = color_start;
    p->color_end = color_end;
    p->lifespan = p->lifespan_now = lifespan;
    p->size_start = size_start;
    p->size_end = size_end;    
  }
}

static void
lit_game_update_particles(lit_game_t* g, f32_t dt) {
  lit_particle_pool_t* ps = &g->particles;
  for(u32_t particle_id = 0; 
      particle_id < ps->particle_count; ) 
  {
    lit_particle_t* p = ps->particles + particle_id;
    if (p->lifespan_now <= 0.f) {
      // Copy last item to current item
      (*p) = ps->particles[ps->particle_count-- - 1]; 

    }
    else { 
      p->lifespan_now -= dt;
      p->pos += p->vel * dt;
      ++particle_id;
    }

  }
}
static void
lit_game_render_particles(lit_game_t* g) {
  lit_particle_pool_t* ps = &g->particles;

  // Render particles
  for(u32_t particle_id = 0; 
      particle_id < ps->particle_count;
      ++particle_id) 
  {
    lit_particle_t* p = ps->particles + particle_id;

    f32_t lifespan_ratio = 1.f -  p->lifespan_now / p->lifespan;

    rgba_t color = {0};
    color.r = f32_lerp(p->color_start.r, p->color_end.r, lifespan_ratio);  
    color.g = f32_lerp(p->color_start.g, p->color_end.g, lifespan_ratio);  
    color.b = f32_lerp(p->color_start.b, p->color_end.b, lifespan_ratio);  
    color.a = f32_lerp(p->color_start.a, p->color_end.a, lifespan_ratio);  

    v2f_t size = {0};
    size.w = f32_lerp(p->size_start.w , p->size_end.w, lifespan_ratio);
    size.h = f32_lerp(p->size_start.h , p->size_end.h, lifespan_ratio);

    gfx_push_asset_sprite(gfx, &lit->assets, g->filled_circle_sprite, p->pos, size, color);
    gfx_advance_depth(gfx);
  }
}


//
// Sensors
//
static lit_game_sensor_t* 
lit_game_push_sensor(lit_game_t* g, f32_t pos_x, f32_t pos_y, u32_t target_color) 
{
  assert(g->selected_sensor_group_id != array_count(g->sensor_groups));
  assert(g->sensor_count < array_count(g->sensors));
  lit_game_sensor_t* s = g->sensors + g->sensor_count++;
  s->pos.x = pos_x;
  s->pos.y = pos_y;
  s->target_color = target_color;
  s->current_color = 0;
  s->group_id = g->selected_sensor_group_id;

  g->sensor_groups[s->group_id].sensor_count++;

  return s;
}


static void
lit_game_push_rotating_sensor(
    lit_game_t* g, 
    f32_t pos_x, 
    f32_t pos_y, 
    v2f_t* origin,
    f32_t speed, 
    u32_t target_color)
{
  auto* sensor = lit_game_push_sensor(g, pos_x, pos_y, target_color);
  lit_game_animator_push_rotate_point(g, &sensor->pos, origin, speed);

}



static void
lit_game_push_patrolling_sensor_waypoint(
    lit_game_t* g, 
    f32_t pos_x, 
    f32_t pos_y) 
{
  assert(g->selected_animator_for_sensor);
  lit_game_animator_push_patrol_point_waypoint(g->selected_animator_for_sensor, pos_x, pos_y);
}


static void
lit_game_begin_patrolling_sensor(lit_game_t* g, f32_t pos_x, f32_t pos_y, u32_t target_color, f32_t duration_per_waypoint) 
{
  assert(!g->selected_animator_for_sensor);

  auto* sensor = lit_game_push_sensor(g, pos_x, pos_y, target_color);
  g->selected_animator_for_sensor = 
    lit_game_animator_begin_patrol_point(g, duration_per_waypoint, &sensor->pos);

  //lit_game_push_patrolling_sensor_waypoint(g, pos_x, pos_y);

}

static void
lit_game_end_patrolling_sensor(lit_game_t* g) {

  assert(g->selected_animator_for_sensor);
  lit_game_animator_end_patrol_point(g->selected_animator_for_sensor);

  g->selected_animator_for_sensor = nullptr;
}

static void 
lit_game_begin_sensor_group(
    lit_game_t* m, 
    lit_game_sensor_callback_t* callback,
    void* context = nullptr)
{
  assert(m->sensor_group_count < array_count(m->sensor_groups));
  m->selected_sensor_group_id = m->sensor_group_count++;

  lit_game_sensor_group_t* group = m->sensor_groups + m->selected_sensor_group_id;
  group->callback = callback;
  group->sensor_count = 0;
  group->context = context;
}

static void 
lit_game_end_sensor_group(lit_game_t* m) {
  m->selected_sensor_group_id = array_count(m->sensor_groups);
}

static void 
lit_game_update_sensors(lit_game_t* g, f32_t dt) 
{
  lit_particle_pool_t* particles = &g->particles;
  rng_t* rng = &g->rng; 

  // This is an array of activated sensors per sensor_group
  u32_t* activated = arena_push_arr_zero(u32_t, &lit->frame_arena, g->sensor_group_count);

  u32_t total_triangles = 0;

  // Go through each sensor and update what lights are on it
  for(u32_t sensor_index = 0; sensor_index < g->sensor_count; ++sensor_index)
  {
    lit_game_sensor_t* sensor = g->sensors + sensor_index;
    u32_t current_color = 0x0000000;

    // For each light, for each triangle, add light
    for(u32_t light_index = 0; light_index < g->light_count; ++light_index)
    {
      lit_game_light_t* light = g->lights +light_index;

      lit_profile_begin(sensor_light_overlap);
      for(u32_t tri_index = 0; tri_index < light->triangle_count; ++tri_index)
      {
        lit_game_light_triangle_t* tri = light->triangles +tri_index;
        if (bonk_tri2_pt2(tri->p0, tri->p1, tri->p2, sensor->pos)) 
        {
          current_color += light->color >> 8 << 8; // ignore alpha
          break; // ignore the rest of the triangles
        }
      }
      lit_profile_end(sensor_light_overlap);

      total_triangles += light->triangle_count;
    }

    // Sensor color check
    sensor->current_color = current_color;
    if ((sensor->current_color & LIT_SENSOR_COLOR_MASK) == 
        (sensor->target_color & LIT_SENSOR_COLOR_MASK)) 
    {
      ++activated[sensor->group_id];
    }

    // Particle emission check
    sensor->particle_cd -= dt;
    if (sensor->particle_cd <= 0.f) 
    {
      sensor->particle_cd = LIT_SENSOR_PARTICLE_CD;
      v2f_t rand_dir = rng_unit_circle(rng);
      v2f_t particle_vel = v2f_scale(rand_dir, LIT_SENSOR_PARTICLE_SPEED); 
      rgba_t target_color = rgba_hex(sensor->target_color); 

      rgba_t start_color = target_color;
      start_color.a = 1.f;

      rgba_t end_color = target_color;
      end_color.a = 0.f;

      v2f_t size_start = v2f_set(LIT_SENSOR_PARTICLE_SIZE, LIT_SENSOR_PARTICLE_SIZE);
      v2f_t size_end = v2f_zero();

      lit_spawn_particle(
          g, 
          1.f,
          sensor->pos,
          particle_vel,
          start_color,
          end_color,
          size_start,
          size_end);
    }
  }

  // For each sensor group, we find which one is actually activated
  for_cnt(group_index, g->sensor_group_count) {
    lit_game_sensor_group_t* group = g->sensor_groups + group_index;
    if (group->sensor_count == activated[group_index]) {
      // We found a group that can activate the callback
      group->callback(g, group->context);
    }
  }

  inspector_add_u32(&lit->inspector, str8_from_lit("total_triangles"), total_triangles);
}


static void 
lit_game_render_sensors(lit_game_t* g) {

  v2f_t size = v2f_set(LIT_SENSOR_RADIUS,LIT_SENSOR_RADIUS)*2;
  for(u32_t sensor_index = 0; sensor_index < g->sensor_count; ++sensor_index)
  {
    lit_game_sensor_t* sensor = g->sensors + sensor_index;
    gfx_push_asset_sprite(gfx, &lit->assets, g->filled_circle_sprite, sensor->pos, size, rgba_hex(sensor->target_color));
    gfx_advance_depth(gfx);
  }
}

static void
lit_game_generate_light(lit_game_t* g) {
  for(u32_t light_index = 0; light_index < g->light_count; ++light_index)
  {
    lit_game_light_t* light = g->lights + light_index;
    lit_gen_light_intersections(light, g->edges, g->edge_count, &lit->frame_arena);

#if LIT_DEBUG_INTERSECTIONS
    // Generate debug lines
    for (u32_t intersection_index = 0;
        intersection_index < light->intersection_count;
        ++intersection_index)
    {
      v2f_t p0 = light->pos;
      v2f_t p1 = light->intersections[intersection_index].pt;
      gfx_push_line(gfx, p0, p1, 1.f, rgba_hex(0xFFFFFFFF));
    }
#endif
  }
}
