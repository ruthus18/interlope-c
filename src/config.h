#pragma once
#include <stdint.h>
#include <cglm/cglm.h>


#define ENGINE_VERSION  "0.0.1a"

/* Window (User)*/
// #define WINDOW_WIDTH 1366
// #define WINDOW_HEIGHT 768
#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 1050
#define WINDOW_XPOS 0
#define WINDOW_YPOS 0
#define WINDOW_BORDER false
#define WINDOW_FULLSC true
#define WINDOW_VSYNC false

/* Window (System)*/
#define WINDOW_TITLE "Interlope Engine"
#define WINDOW_BG_COLOR (f32)29 / 255, (f32)32 / 255, (f32)33 / 255, 1.0
#define WINDOW_MAX_FRAMERATE 120.0

/* GFX */
#define GFX_WIREFRAME_MODE false

/* Paths & Dirs */
#define DIR_SHADERS "shaders/"
#define DIR_MODELS "assets/models/"
#define DIR_TEXTURES "assets/textures/"

/* Input */
#define MOUSE_SENSITIVITY 120.0

/* Camera */
#define CAMERA_DEFAULT_FOV 90.0
#define CAMERA_MOVEMENT_SPEED 4


/* ------------------------------------------------------------------------- */
/* Debugging */

#define __DEBUG__PRINT_FPS false
#define __DEBUG__PRINT_TIME_UPDATE false
#define __DEBUG__LOG_CAMERA_ROTATION false
#define __DEBUG__LOG_CAMERA_POSITION false
#define __DEBUG__LOG_CAMERA_POSITION_DELTA false


/* ------------------------------------------------------------------------- */
/* Fancy functions for easy living */

#define len(x) ((sizeof(x)/sizeof(0[x])) / ((u64)(!(sizeof(x) % sizeof(0[x])))))
