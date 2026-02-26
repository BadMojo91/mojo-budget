#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>

#define CIMGUI_USE_SDL2
#define CIMGUI_USE_OPENGL2
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "cimgui_impl.h"

#include <GL/gl.h>

#include "budget/bill.h"
#include "sdl/im_menubar.h"

SDL_Window *window = NULL;
SDL_GLContext *gl_context = NULL;

int main(int argc, char *argv[])
{
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    printf("Failed to initialize SDL: %s\n", SDL_GetError());
    return 1;
  }

  window = SDL_CreateWindow(
      "Budget App", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 800,
      SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  if (!window)
  {
    printf("Failed to create window: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

  gl_context = SDL_GL_CreateContext(window);
  if (!gl_context)
  {
    printf("Failed to create OpenGL context: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  igCreateContext(NULL);

  ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL2_Init();

  ImGuiIO *ioptr = igGetIO_Nil();
 
  
  entryMap = LoadEntryMap("default.bud");

  bool running = true;
  while (running)
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      ImGui_ImplSDL2_ProcessEvent(&event);

      if (event.type == SDL_QUIT ||
          (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
      {
        running = false;
      }
    }

    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    igNewFrame();

    DrawUI(&running);

    igRender();
    SDL_GL_MakeCurrent(window, gl_context);
    glViewport(0, 0, 800, 600);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL2_RenderDrawData(igGetDrawData());
    SDL_GL_SwapWindow(window);
  }
  SaveEntryMap("default.bud", entryMap, FILETYPE_BUD);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
