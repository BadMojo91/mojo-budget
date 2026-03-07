#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"

#ifdef _WIN32
  #include <windows.h>
  #include <direct.h>
  #include <sys/stat.h>
  #define MKDIR(path) _mkdir(path)
  #define PATH_SEP "\\"
#else
  #include <sys/stat.h>
  #define MKDIR(path) mkdir(path, 0755)
  #define PATH_SEP "/"
#endif

Config config;

static char config_dir[1024] = { 0 };
const char* GetConfigDir(void)
{
  if (config_dir[0] != '\0')
    return config_dir;

#ifdef _WIN32
  const char* appdata = getenv("APPDATA");
  if (!appdata)
  {
    fprintf(stderr, "Could not determine AppData directory\n");
    return NULL;
  }
  snprintf(config_dir, sizeof(config_dir), "%s\\mojo-budget", appdata);
#else
  const char* home = getenv("HOME");
  if (!home)
  {
    fprintf(stderr, "Could not determine home directory\n");
    return NULL;
  }
  snprintf(config_dir, sizeof(config_dir), "%s/.config/mojo-budget", home);
#endif

  struct stat st = { 0 };
  if (stat(config_dir, &st) == -1)
  {
    if (MKDIR(config_dir) == -1)
    {
      fprintf(stderr, "Could not create config directory: %s\n", config_dir);
      return NULL;
    }
  }

  return config_dir;
}

static char config_path[1024] = { 0 };
const char* GetConfigFilePath()
{
  if (config_path[0] != '\0')
    return config_path;

  snprintf(config_path, sizeof(config_path), "%s/config.cfg", GetConfigDir());
  return config_path;
}

Config CreateDefaultConfig(void)
{
  Config cfg;

  cfg.window_height = DEFAULT_WINDOW_HEIGHT;
  cfg.window_width = DEFAULT_WINDOW_WIDTH;
  return cfg;
}

void ReadConfig(Config* cfg)
{
  cfg->window_width = DEFAULT_WINDOW_WIDTH;
  cfg->window_height = DEFAULT_WINDOW_HEIGHT;

  const char* path = GetConfigFilePath();
  FILE* file = fopen(path, "r");
  if (file == NULL)
  {
    printf("Config not found, creating default config.cfg\n");
    SaveConfig(cfg);

    file = fopen(path, "r");
    if (file == NULL)
    {
      printf("Error creating config.cfg\n");
      return;
    }
  }

  const char* trimmedPath = TrimHomePath(path);
  printf("Reading config: %s\n", trimmedPath);
  char line[256];

  while (fgets(line, sizeof(line), file))
  {
    line[strcspn(line, "\r\n")] = 0;

    char* token = strchr(line, '=');
    if (!token)
      continue;
    *token = '\0';

    const char* key = line;
    const char* value = token + 1;

    if (strcmp(key, "windowWidth") == 0)
      cfg->window_width = atoi(value);
    else if (strcmp(key, "windowHeight") == 0)
      cfg->window_height = atoi(value);
  }
  fclose(file);
}

int SaveConfig(Config* cfg)
{
  char path[1024];
  snprintf(path, sizeof(path), "%s/config.cfg", GetConfigDir());
  FILE* file = fopen(path, "w");
  if (file == NULL)
  {
    fprintf(stderr, "Could not save config: %s\n", path);
    perror("Details");
    return 0;
  }

  fprintf(file, "windowWidth=%d\n", cfg->window_width);
  fprintf(file, "windowHeight=%d\n", cfg->window_height);
  fclose(file);

  printf("Successfully saved config: %s\n", path);
  printf("[Window]\n%dx%d\n", cfg->window_width, cfg->window_height);
  return 1;
}

