#include <math.h>
#include <stdio.h>
#define __USE_XOPEN_EXTENDED  // FIXME
#include <unistd.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "time.h"

#include "core/config.h"
#include "core/types.h"


static f64 current_time = 0.0;       // GetTime value from GLFW
static f64 last_time;
static f64 dt;

static f64 timer_sec = 0.0;
static i32 nbframes = 0;                // num of frames after recent timer reset
static i32 fps = 0;                     // last record of nbframes
static bool second_passed = false;      // mark true on every 1 sec frame (timer)


void time_update() {
    last_time = current_time;
    current_time = glfwGetTime();

    dt = current_time - last_time;
    nbframes++;
    timer_sec += dt;

    /* Update vars and reset after 1.0 sec */
    if (timer_sec >= 1.0) {
        second_passed = true; 
        fps = ceil(nbframes / timer_sec);
        timer_sec -= 1.0;
        nbframes = 0;
    }
    else if (second_passed)
        second_passed = false;
}

f64 time_get_dt() { return dt; }
int time_get_fps() { return fps; }


/* Temporary solution, not thread-safe */
void time_limit_framerate() {
    if (Config.WINDOW_MAX_FRAMERATE == 0.0)  return;

    f64 min_frame_duration = 1.0 / Config.WINDOW_MAX_FRAMERATE;
    f64 frame_duration = glfwGetTime() - current_time;

    if (frame_duration < min_frame_duration) {
        usleep((min_frame_duration - frame_duration) * 1000000);
    }
}
