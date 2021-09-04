#include <GL/glew.h>
#include <GL/gl.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"
#include "SDL2/SDL_syswm.h"
#include "window.h"
#include <stdio.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <math.h>

static SDL_Window *window;
static SDL_GLContext context;
static Vec2 windowSize = {0,0};
static Vec2 viewportSize = {0,0};
static Vec2 originalViewportSize = {0,0};
static int breakLoop = 0;
static int deltaTime = 0;
static int maxDeltaTime = 100;
static SDL_GameController *controller = NULL;

int Window_Open(const char *title, int posx, int posy, int width, int height, int fs){

    SDL_Init(SDL_INIT_GAMECONTROLLER);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    int k;
    for(k = 0; k < SDL_NumJoysticks(); k++){
        if(SDL_IsGameController(k)){
            controller = SDL_GameControllerOpen(k);
            if(controller) break;
        }
    }

    window = SDL_CreateWindow(
        title,
        posx,
        posy,
        width,
        height,
        SDL_WINDOW_OPENGL | fs
    );
    context = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(0);

    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK) {
        printf("Glew Init Failed\n");
        return 0;
    }

    windowSize.x = width;
    windowSize.y = height;
    if(viewportSize.x <= 0) viewportSize.x = width;
    if(viewportSize.y <= 0) viewportSize.y = height;

    SDL_SetRelativeMouseMode(1);
    // SDL_ShowCursor(0);
    SDL_WarpMouseInWindow(window, windowSize.x/2, windowSize.y/2);

    return 1;
}

static void Window_CleanUp(){

    // if(controller) SDL_GameControllerClose(controller);
	SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
	SDL_Quit();
}

void Window_Close(){
	Window_CleanUp();
	breakLoop = 0;
}

void Window_SetMousePos(int x, int y){ SDL_WarpMouseInWindow(window, x, y); }
int Window_GetDeltaTime(){ return deltaTime; }
int Window_GetTicks(){ return SDL_GetTicks(); }
int Window_MaxDeltaTime(){ return maxDeltaTime; }
int Window_GetWindowHeight(){ return windowSize.y; }
int Window_GetWindowWidth(){ return windowSize.x; }
int Window_GetViewportWidth() { return viewportSize.x; }
int Window_GetViewportHeight() { return viewportSize.y; }
Vec2 Window_GetViewportSize(){return viewportSize; }
Vec2 Window_GetWindowSize(){return windowSize; }

int Window_SetViewportWidth(int w) {
    if(w <= 0) return viewportSize.x;
    if(w > windowSize.x) w = windowSize.x;
    return viewportSize.x = originalViewportSize.x = w;
}

int Window_SetViewportHeight(int h) {
    if(h <= 0) return viewportSize.y;
    if(h > windowSize.y) h = windowSize.y;
    return viewportSize.y = originalViewportSize.y = h;
}

void Window_MainLoop(void (*Update)(), void (*Event)(SDL_Event ), char (*Draw)(), void (*Focused)(),  void (*OnResize)(),
    int display_fps, int stretch){

    SDL_Event event;
    int prevDisplayFPS = SDL_GetTicks();
    int lastTime = SDL_GetTicks();;
    int frames = 0;

    breakLoop = 1;

    while(breakLoop){

        while(SDL_PollEvent(&event)){
            
            if(event.type == SDL_WINDOWEVENT){
                
                if(event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED){
                
                    Focused();
                
                } else if(stretch && event.window.event == SDL_WINDOWEVENT_RESIZED){

                    windowSize.x = event.window.data1;
                    windowSize.y = event.window.data2;

                    glViewport(0,0,windowSize.x, windowSize.y);

                    OnResize();
                }
            }
            else if(event.type == SDL_QUIT){
                Window_Close();
                break;
            }

            Event(event);
        }

        if(!breakLoop) break;

        int currTime = SDL_GetTicks();
        deltaTime = currTime - lastTime;

        if(deltaTime > (1.0/10) * 1000){
            lastTime = currTime;
            continue;
        } 

        if(deltaTime > 0){
            lastTime = currTime;
            Update();
        }
    
        if(Draw()) SDL_GL_SwapWindow(window);

        frames++;

        if(display_fps){
            int currTime = SDL_GetTicks();
            if(currTime - prevDisplayFPS > 1000 ){
                printf("%f ms\n", ((float)currTime - (float)prevDisplayFPS) / frames );
                printf("%i fps\n", frames );
                prevDisplayFPS = currTime;
                frames = 0;
            }
        }
    }
}