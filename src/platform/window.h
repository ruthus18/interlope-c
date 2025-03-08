#pragma once
#include <GLFW/glfw3.h>


typedef GLFWwindow Window;

void window_init();
void window_destroy();
Window* window_get();
void window_poll_events();
void window_swap_buffers();
