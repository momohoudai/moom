// This file and game_gfx.h contain structs that need to be 
// initialized by the OS and passed to the main Game_Update() 
// function. 
#ifndef GAME_PLATFORM_H
#define GAME_PLATFORM_H


//~NOTE(Momo): Platform API
typedef void PF_HotReloadFn(); // trigger hot reloading of game code
typedef void* PF_AllocFn(UMI size); // allocate memory
typedef void PF_FreeFn(void* ptr);     // frees memory

typedef struct {
  PF_HotReloadFn* hot_reload;
  PF_AllocFn* alloc;
  PF_FreeFn* free;
} PF;



//~ NOTE(Momo): Input API
// TODO(Momo): Perhaps we want something a lot more robust?
// But that will probably require some insane enum
// Or maybe we can put all these into Platform API?

struct InputButton{
  B32 before;
  B32 now;
};

static B32 IsPoked(InputButton) ;
static B32 IsReleased(InputButton);
static B32 IsDown(InputButton);
static B32 IsHeld(InputButton);

struct Input{
  InputButton buttons[4];
  struct {
    InputButton button_up;
    InputButton button_down;
    InputButton button_left;
    InputButton button_right;
  };  
  
  V2F32 design_mouse_pos;
  V2U32 screen_mouse_pos;
  V2U32 render_mouse_pos;
  
};

static void Update(Input* input);


//- Implementation
static void Update(Input* input) {
  for (U32 i = 0; i < ArrayCount(input->buttons); ++i) {
    input->buttons[i].before = input->buttons[i].now;
  }
}



// before: 0, now: 1
static B32 
IsPoked(InputButton b) {
  return !b.before && b.now;
}

// before: 1, now: 0
static B32
IsReleased(InputButton b) {
  return b.before && !b.now;
}


// before: X, now: 1
static B32
IsDown(InputButton b){
  return b.now;
}

// before: 1, now: 1
static B32
IsHeld(InputButton b) {
  return b.before && b.now;
}

//~ NOTE(Momo): Game API
// Returns true if game is done
typedef B32 Game_UpdateFn(PF* pf,
                          Input* input,
                          Gfx* gfx,
                          F32 dt);

struct Game_API {
  Game_UpdateFn* update;
  
};




#endif //GAME_PLATFORM_H
