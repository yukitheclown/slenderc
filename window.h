#ifndef WINDOW_DEF
#define WINDOW_DEF
#include "SDL2/SDL.h"
#include "math.h"

int Window_Open(const char * title, int posx, int posy, int width, int height, int fs);
void Window_Close();
void Window_MainLoop(void (*Update)(), void (*Event)(SDL_Event), char (*Draw)(), void (*Focused)(), void (*OnResize)(), int display_fps, int stretch);
int Window_GetDeltaTime();
int Window_GetTicks();
int Window_MaxDeltaTime();
int Window_GetWindowWidth();
int Window_GetWindowHeight();
int Window_GetWidth();
int Window_GetHeight();
Vec2 Window_GetViewportSize();
Vec2 Window_GetWindowSize();
int Window_GetViewportWidth();
int Window_GetViewportHeight();
int Window_SetViewportWidth(int w);
int Window_SetViewportHeight(int h);
void Window_SetMousePos(int x, int y);

#endif