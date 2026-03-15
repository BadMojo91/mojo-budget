#ifndef THEME_H
#define THEME_H

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include <stddef.h>

#define THEME_NAME_MAX 256

typedef struct {
  char  name[THEME_NAME_MAX];
  ImVec4 colors[ImGuiCol_COUNT];
} Theme;

/* Premade theme names */
#define THEME_DARK                    "Dark"
#define THEME_LIGHT                   "Light"
#define THEME_CLASSIC                 "Classic"
#define THEME_CATPPUCCIN_LATTE        "Catppuccin Latte"
#define THEME_CATPPUCCIN_FRAPPE       "Catppuccin Frappe"
#define THEME_CATPPUCCIN_MACCHIATO    "Catppuccin Macchiato"
#define THEME_CATPPUCCIN_MOCHA        "Catppuccin Mocha"
#define THEME_NORD                    "Nord"
#define THEME_DRACULA                 "Dracula"
#define THEME_HIGH_CONTRAST_AMBER     "High Contrast Amber"
#define THEME_HIGH_CONTRAST_CYAN      "High Contrast Cyan"
#define THEME_HIGH_CONTRAST_LIME      "High Contrast Lime"
#define THEME_PSYCHEDELIC_NEON        "Psychedelic Neon"

#define THEME_CATPPUCCIN THEME_CATPPUCCIN_MOCHA

/* Returns 1 if name matches one of the built-in premade themes. */
int  IsPremadeTheme(const char *name);

/* Applies a premade theme to the current ImGui style by name.
   Falls back to Dark if name is not recognized. */
void ApplyPremadeTheme(const char *name);

/* Applies colors from a custom Theme struct to the current ImGui style. */
void ApplyCustomTheme(const Theme *t);

/* Fills buf with the path to the themes subdirectory, creating it if needed. */
void GetThemesDir(char *buf, size_t sz);

/* Scans dir for *.theme files, allocates and fills *out with parsed themes.
   *count is set to the number loaded. Caller must call FreeCustomThemes.
   Returns the number of themes loaded. */
int  LoadCustomThemes(const char *dir, Theme **out, int *count);

/* Saves the current ImGui style as a .theme file in dir with the given name.
   Returns 1 on success, 0 on failure. */
int  SaveCustomTheme(const char *dir, const char *name);

/* Deletes the .theme file for the given theme name in dir.
   Returns 1 on success, 0 on failure. */
int  DeleteCustomTheme(const char *dir, const char *name);

/* Frees a themes array returned by LoadCustomThemes. */
void FreeCustomThemes(Theme *themes);

/* Convenience: applies a theme by name, trying premade first, then custom
   themes from themesDir.  Safe to call with NULL themesDir (skips custom). */
void ApplyThemeByName(const char *themesDir, const char *name);

#endif /* THEME_H */
