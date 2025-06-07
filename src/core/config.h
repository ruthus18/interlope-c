#pragma once
#include <stdint.h>

#include <cglm/cglm.h>


#define ENGINE_VERSION  "0.0.1a"

/* --- Window (User) --- */
// #define WINDOW_WIDTH 1366
// #define WINDOW_HEIGHT 768
// #define WINDOW_FULLSC false
#define WINDOW_WIDTH    1680
#define WINDOW_HEIGHT   1050
#define WINDOW_FULLSC   true
#define WINDOW_XPOS     0
#define WINDOW_YPOS     0
#define WINDOW_BORDER   false
#define WINDOW_VSYNC    false

/* --- Window (System) --- */
#define WINDOW_TITLE            "Interlope Engine"
#define WINDOW_BG_COLOR         (f32)29 / 255, (f32)32 / 255, (f32)33 / 255, 1.0
#define WINDOW_MAX_FRAMERATE    120.0

/* --- GFX --- */
#define GFX_WIREFRAME_MODE  false

/* --- Paths & Dirs --- */
#define DIR_SHADERS     "shaders/"
#define DIR_MESHES      "assets/meshes/"
#define DIR_TEXTURES    "assets/textures/"

/* --- Camera --- */
#define CAMERA_DEFAULT_FOV      75.0
#define CAMERA_MOVEMENT_SPEED   6
#define CAMERA_SENSITIVITY      130.0

/* --- World --- */
#define WORLD_OBJDB_PATH        "data/objects.toml"
#define WORLD_INIT_SCENE_PATH   "data/scenes/test.toml"

/* ------------------------------------------------------------------------- */
/* --- Debugging --- */

#define __DEBUG__PRINT_FPS                  false
#define __DEBUG__PRINT_TIME_UPDATE          false
#define __DEBUG__LOG_CAMERA_ROTATION        false
#define __DEBUG__LOG_CAMERA_POSITION        false
#define __DEBUG__LOG_CAMERA_POSITION_DELTA  false
