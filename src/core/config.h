#pragma once
#include <stdbool.h>


#define ENGINE_VERSION  "0.0.1a"

typedef struct {
    char WINDOW_TITLE[64];
    int WINDOW_WIDTH;
    int WINDOW_HEIGHT;
    bool WINDOW_FULLSC;
    int WINDOW_XPOS;
    int WINDOW_YPOS;
    bool WINDOW_BORDER;
    bool WINDOW_VSYNC;
    double WINDOW_MAX_FRAMERATE;
    
    bool GRAPHICS_WIREFRAME;
    
    char DIR_SHADERS[64];
    char DIR_MESHES[64];
    char DIR_TEXTURES[64];
    char PATH_OBJECTS_DATA[64];
    char PATH_SCENES_DATA[64];
} _Config;

extern _Config Config;

void config_load(const char* config_path);
