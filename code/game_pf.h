// This file and GFX.h contain structs that need to be 
// initialized by the OS and passed to the main Game_Update() 
// function. 
#ifndef GAME_PLATFORM_H
#define GAME_PLATFORM_H

//~Platform File API
enum Platform_File_Path {
  PLATFORM_FILE_PATH_EXE,
  PLATFORM_FILE_PATH_USER,
  PLATFORM_FILE_PATH_CACHE,
  
};

// Maybe for 'overwrite' or creating a new file, we 
// use a compl
enum Platform_File_Access {
  PLATFORM_FILE_ACCESS_READ,
  PLATFORM_FILE_ACCESS_OVERWRITE,
};

struct Platform_File {
  B32 error;
  void* platform_data; // pointer for platform's usage
};
typedef Platform_File  
Platform_Open_File(const char* filename,
                   Platform_File_Access file_access,
                   Platform_File_Path file_path);

typedef void Platform_Close_File(Platform_File* file);
typedef void Platform_Read_File(Platform_File* file, UMI size, UMI offset, void* dest);
typedef void Platform_Write_File(Platform_File* file, UMI size, UMI offset, void* src);

//~Platform multithreaded work API
typedef void Platform_Work_Callback(void* data);
typedef void Platform_Add_Work(Platform_Work_Callback callback, void* data);
typedef void Platform_Complete_All_Work();


//~Other platform API
typedef void  Platform_Hot_Reload(); // trigger hot reloading of game code
typedef void  Platform_Shutdown(); // trigger shutdown of application
typedef void* Platform_Alloc(UMI size); // allocate memory
typedef void  Platform_Free(void* ptr);     // frees memory
typedef void  Platform_Set_Aspect_Ratio(U32 width, U32 height); // sets aspect ratio of game


struct Platform_API {
  Platform_Hot_Reload* hot_reload;
  Platform_Shutdown* shutdown;
  Platform_Alloc* alloc;
  Platform_Free* free;
  Platform_Set_Aspect_Ratio* set_aspect_ratio;
  
  Platform_Open_File* open_file;
  Platform_Read_File* read_file;
  Platform_Write_File* write_file;
  Platform_Close_File* close_file;
  
  Platform_Add_Work* add_work;
  Platform_Complete_All_Work* complete_all_work;
};
extern Platform_API g_platform;



//~Input API
// NOTE(Momo): Game_Input is not just 'controllers'.
// It is filled with 'things the game need to respond to' 
// so hence it's name and also why delta time is in there


struct Game_Input_Button {
  B32 before;
  B32 now; 
};

static B32 is_poked(Game_Input_Button) ;
static B32 is_released(Game_Input_Button);
static B32 is_down(Game_Input_Button);
static B32 is_held(Game_Input_Button);



struct Game_Input {
  Game_Input_Button buttons[4];
  struct {
    Game_Input_Button button_up;
    Game_Input_Button button_down;
    Game_Input_Button button_left;
    Game_Input_Button button_right;
  };  
  
  V2 design_mouse_pos;
  V2U screen_mouse_pos;
  V2U render_mouse_pos;
  
  F32 seconds_since_last_frame; //aka dt
};

void update(Game_Input_Button button);




//~ NOTE(Momo): Game API
// For things that don't change from the platform after setting it once
struct Gfx_Texture_Queue;
struct Game_Memory {
  struct Game_State* state; // pointer for game memory usage
  
  Platform_API platform_api;
  Gfx_Texture_Queue* texture_queue;
};

struct Game_Render_Commands;
typedef void Game_Update_Fn(Game_Memory* memory,
                            Game_Input* input,
                            Game_Render_Commands* render_commands);
#define DECLARE_GAME_UPDATE_FN(name) 

// To be called by platform
struct Game_API {
  Game_Update_Fn* update;
};


#include "game_pf.cpp"

#endif //GAME_PLATFORM_H
