#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>

#include <GL/gl.h>
#include <SDL2/SDL.h>

#define CIMGUI_USE_SDL2
#define CIMGUI_USE_OPENGL2
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "cimgui_impl.h"

#include "ui_core.h"

#include "../core/bill.h"
#include "../core/config.h"
#include "../core/export.h"
#include "../core/utility.h"
SDL_Window *window = NULL;
SDL_GLContext *gl_context = NULL;
static char imgui_ini_path[1024] = {0};

Config cfg;

void ShowHelpText()
{
  printf("Mojo Budget - Command Line Utility\n");
  printf("Usage:\n");
  printf("  mojo-budget [OPTIONS]\n\n");
  printf("Options:\n");
  printf("  -e <src> <dest>   Export budget file to another format (txt, csv, "
         "tsv, xlsx)\n");
  printf("  -h                Show this help message\n");
}

int Export(const char *src, const char *dest)
{
  if (!src)
  {
    printf("Source file is required for export.\n");
    return 1;
  }

  if (strcmp(GetExt(src), ".bud") != 0)
  {
    printf("%s is not a budget file, needs the extension *.bud\n", src);
    return 1;
  }

  entryMap = BudgetLoad(src);
  if (entryMap)
  {
    if(dest && dest[0] != '\0')
    {
      const char *destExt = GetExt(dest);
      printf("Exporting %s to %s\n", src, dest);
      if (strcmp(destExt, ".txt") == 0)
      {
        ExportAsTXT(entryMap, dest);
      }
      else if (strcmp(destExt, ".csv") == 0)
      {
        ExportAsCSV(entryMap, dest);
      }
      else if (strcmp(destExt, ".tsv") == 0)
      {
        ExportAsTSV(entryMap, dest);
      }
      else if (strcmp(destExt, ".xlsx") == 0)
      {
        ExportAsXLSX(entryMap, dest);
      }
      else
      {
        printf("Unsupported export format: %s\n", destExt);
        return 1;
      }
    }
    else
    {
      const char *path = SetExt(src, "txt");
      ExportAsTXT(entryMap, path);
    }
  }
  else
  {
    printf("Could not load: %s\n", src);
    return 1;
  }
  return 0;
}

int main(int argc, char *argv[])
{

  if (argc > 1)
  {
    for (int i = 1; i < argc; i++)
    {
      if (argv[i][0] == '-' && argv[i][1] != '\0')
      {
        char option = argv[i][1];
        switch (option)
        {
        case 'h':
          ShowHelpText();
          break;
        case 'e':
          if (i + 2 < argc)
          {
            const char *src = argv[i + 1];
            const char *dest = argv[i + 2];
            if (Export(src, dest) != 0)
            {
              return 1;
            }
          }
          else if (i + 1 < argc)
          {
            const char *src = argv[i + 1];
            if (Export(src, NULL) != 0)
            {
              return 1;
            }
          }
          break;
        }
      }
    }
    return 0;
  }

  ReadConfig(&cfg);

  int width = cfg.window_width;
  int height = cfg.window_height;

  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    printf("Failed to initialize SDL: %s\n", SDL_GetError());
    return 1;
  }
  window = SDL_CreateWindow(
      "Mojo Budget", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width,
      height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
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
  const char *config_dir = GetConfigDir();
  if (config_dir)
  {
    snprintf(imgui_ini_path, sizeof(imgui_ini_path), "%s/imgui.ini",
             config_dir);
    ioptr->IniFilename = imgui_ini_path;
  }

  char defaultPath[1024];
  snprintf(defaultPath, sizeof(defaultPath), "%s/default.bud", GetConfigDir());

  entryMap = BudgetLoad(defaultPath);

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
    // glViewport(0, 0, 800, 600);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL2_RenderDrawData(igGetDrawData());
    SDL_GL_SwapWindow(window);

    SDL_Delay(16);
  }

  SDL_GetWindowSize(window, &cfg.window_width, &cfg.window_height);
  SaveConfig(&cfg);

  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
