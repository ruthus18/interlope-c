#pragma once
#include <stdbool.h>
#include <stdint.h>

#define sconst_ static const


const char* ENGINE_VERSION = "0.0.1";

const int  WINDOW_WIDTH = 1366;
const int  WINDOW_HEIGHT = 768;
const int  WINDOW_XPOS = 0;
const int  WINDOW_YPOS = 0;
const bool  WINDOW_BORDER = false;
const bool  WINDOW_FULLSC = false;

const char* WINDOW_TITLE = "Interlope Engine";
const bool  WINDOW_VSYNC = true;

const char* DIR_SHADERS = "shaders/";
const char* DIR_ASSETS = "assets/";
