#pragma once
#include <stdint.h>
#include <cglm/cglm.h>


#define ENGINE_VERSION  "0.0.1"

/* Window (User)*/
#define WINDOW_WIDTH 1366
#define WINDOW_HEIGHT 768
#define WINDOW_XPOS 0
#define WINDOW_YPOS 0
#define WINDOW_BORDER false
#define WINDOW_FULLSC false
#define WINDOW_VSYNC true

/* Window (System)*/
#define WINDOW_TITLE "Interlope Engine"
#define WINDOW_BG_COLOR (float)29 / 255, (float)32 / 255, (float)33 / 255, 1.0

/* GFX */
#define GFX_WIREFRAME_MODE true

/* Paths & Dirs */
#define DIR_SHADERS "shaders/"
#define DIR_ASSETS "assets/"
#define DIR_MESHES "assets/meshes/"

/* Input */
#define MOUSE_SENSITIVITY 150.0

/* Camera */
#define CAMERA_DEFAULT_FOV 75.0
#define CAMERA_MOVEMENT_SPEED 5


/* ------------------------------------------------------------------------- */
/* Debugging */

#define __DEBUG__PRINT_FPS false
#define __DEBUG__PRINT_TIME_UPDATE false
#define __DEBUG__LOG_CAMERA_ROTATION false
#define __DEBUG__LOG_CAMERA_POSITION false
#define __DEBUG__LOG_CAMERA_POSITION_DELTA false


/* ------------------------------------------------------------------------- */
/* Fancy functions for easy living */

#define len(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
