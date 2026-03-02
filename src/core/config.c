#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>



static char config_dir[1024] = { 0 };
const char* GetConfigDir(void)
{
  if (config_dir[0] != '\0')
    return config_dir;

  const char* home = getenv("HOME");
  if (!home)
  {
    fprintf(stderr, "Could not determine home directory\n");
    return NULL;
  }

  snprintf(config_dir, sizeof(config_dir), "%s/.config/mojo-budget", home);

  struct stat st = { 0 };
  if (stat(config_dir, &st) == -1)
  {
    if (mkdir(config_dir, 0755) == -1)
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

void ReadConfig(Config* cfg)
{
  cfg->window_width = DEFAULT_WINDOW_WIDTH;
  cfg->window_height = DEFAULT_WINDOW_HEIGHT;
  cfg->bill_table_height = DEFAULT_BILL_TABLE_HEIGHT;
  for (int i = 0; i < NUM_BILL_COLUMNS; i++)
  {
    cfg->bill_column_widths[i] = DEFAULT_COLLUMN_WIDTH;
  }

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
  printf("Reading config: %s\n", path);
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
    else if (strcmp(key, "billColumnWidths") == 0)
    {
      char* p = (char*)value;
      for (int i = 0; i < NUM_BILL_COLUMNS; i++)
      {
        cfg->bill_column_widths[i] = strtof(p, &p);
        if (*p == ',')
          p++;
      }
    }
    else if (strcmp(key, "billTableHeight") == 0)
    {
      cfg->bill_table_height = strtof(value, NULL);
    }
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
  fprintf(file, "billColumnWidths=");
  for (int i = 0; i < NUM_BILL_COLUMNS; i++)
  {
    fprintf(file, "%f", cfg->bill_column_widths[i]);
    if (i < NUM_BILL_COLUMNS - 1)
      fprintf(file, ",");
  }
  fprintf(file, "\n");
  fprintf(file, "billTableHeight=%f\n", cfg->bill_table_height);
  fclose(file);

  printf("Successfully saved config: %s\n", path);
  printf("[Window]\n%dx%d\n", cfg->window_width, cfg->window_height);
  return 1;
}
