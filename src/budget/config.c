#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static char config_dir[1024] = {0};
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
