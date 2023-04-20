static void
lit_init_splash() {
  lit_splash_t* splash = &lit->splash;

  make(asset_match_t, match);
  set_match_entry(match, ASSET_TAG_TYPE_FONT, 0.f, 1.f);
  splash->font = find_best_font(&lit->assets, ASSET_GROUP_TYPE_FONTS, match);
  splash->timer = 1.5f;
}

static void
lit_update_splash() {
  lit_splash_t* splash = &lit->splash;

  splash->timer -= input->delta_time;
 
  gfx_push_text_center_aligned(
      gfx, 
      &lit->assets, 
      splash->font, 
      str8_from_lit("moom"), 
      rgba_set(1.f, 1.f, 1.f, 1.f),
      LIT_WIDTH/2, LIT_HEIGHT/2, 
      128.f);
  gfx_advance_depth(gfx);

  gfx_draw_filled_triangle(gfx, rgba_set(0.f, 1.f, 1.f, 1.f), v2f_set(0,0), v2f_set(0,100), v2f_set(100,100));

  gfx_draw_filled_triangle(gfx, rgba_set(1.f, 0.f, 0.f, 1.f), v2f_set(400, 400), v2f_set(400,500), v2f_set(500,500));

#if 0
  if (splash->timer <= -1.f) {
    lit->next_mode = LIT_MODE_GAME; 
  }
#endif

}


