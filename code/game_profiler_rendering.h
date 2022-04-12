/* date = April 11th 2022 9:11 am */

#ifndef GAME_PROFILER_RENDERING_H
#define GAME_PROFILER_RENDERING_H

// TODO(Momo): Change filename? We are not just rendering. 
// We are also updating.

// TODO(Momo): Change name to update_and_render()
// Might need to seperate this to a 'rendering' file

// TODO(Momo): Add to momo_stat? Looks useful.
struct Stat {
  F64 min;
  F64 max;
  F64 average;
  U32 count;
};

static void
begin_stat(Stat* stat) {
  stat->min = F64_NEG_INFINITY();
  stat->max = F64_INFINITY();
  stat->average = 0.0;
  stat->count = 0;
}

static void
accumulate_stat(Stat* stat, F64 value) {
  ++stat->count;
  if (stat->min > value) {
    stat->min = value;
  }
  if (stat->max < value) {
    stat->max = value;
  }
  stat->average += value;
}

static void
end_stat(Stat* stat) {
  if(stat->count) {
    stat->average /= (F64)stat->count;
  }
  else {
    stat->min = 0.0;
    stat->max = 0.0;
  }
}

static void
render_profiler(Profiler* p, Game_Assets* ga, Game_Render_Commands* cmds) {
  // TODO(Momo): UI coorindates?
  for (U32 entry_index = 0;
       entry_index < p->entry_count;
       ++entry_index) 
  {
    Profiler_Entry* entry = p->entries + entry_index;
    if (entry->function_name ) {
      Stat cycles;
      Stat hits;
      Stat cycles_per_hit;
      
      begin_stat(&cycles);
      begin_stat(&hits);
      begin_stat(&cycles_per_hit);
      
      for (U32 snapshot_index = 0;
           snapshot_index < array_count(entry->snapshots);
           ++snapshot_index)
      {
        
        Profiler_Snapshot * snapshot = entry->snapshots + snapshot_index;
        
        accumulate_stat(&cycles, (F64)snapshot->cycles);
        accumulate_stat(&hits, (F64)snapshot->hits);
        
        F64 cph = 0.0;
        if (snapshot->hits) {
          cph = (F64)snapshot->cycles/(F64)snapshot->hits;
        }
        accumulate_stat(&cycles_per_hit, cph);
      }
      end_stat(&cycles);
      end_stat(&hits);
      end_stat(&cycles_per_hit);
      
      demand_string_builder(sb, 256);
      push_format(sb, 
                  string_from_lit("[%s][%u] %ucy %uh %ucy/h"),
                  entry->function_name,
                  entry->line,
                  (U32)cycles.average,
                  (U32)hits.average,
                  (U32)cycles_per_hit.average);
      
      const F32 font_height = 20.f;
      // Assumes 1600x900
      draw_text(ga, cmds, FONT_DEFAULT, 
                sb->str,
                rgba(0xFFFFFFFF),
                0.f, 
                900.f - font_height * (entry_index+1), 
                font_height,
                0.f);
    }
  }
}

#endif //GAME_PROFILER_RENDERING_H