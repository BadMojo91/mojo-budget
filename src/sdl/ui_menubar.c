#include "ui_core.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "tinyfiledialogs.h"
#include <stb/stb_ds.h>
#include <stdio.h>
#include <sys/stat.h>

#include "../core/bill.h"
#include "../core/config.h"
#include "../core/export.h"
#include "../core/utility.h"
#include "theme.h"

extern bool g_showCalendar;
extern Config cfg;

void DrawFileMenu(bool *running)
{
  if (igBeginMenu("File", true))
  {
    if (igMenuItem_Bool("New", NULL, false, true))
    {
      ClearEntries(&entryMap);
      ClearIncomeEntries(&incomeMap);
    }
    if (igMenuItem_Bool("Open", NULL, false, true))
    {
      const char *path =
          tinyfd_openFileDialog("Open File",               // title
                                "",                        // default path
                                1,                         // filter count
                                (const char *[]){"*.bud"}, // filter
                                "Budget Entry Maps",       // filter description
                                0                          // single select
          );
      if (path)
      {
        entryMap = BudgetLoad(path);
        AddRecentFile(&cfg, path);
        SaveConfig(&cfg);
      }
    }
    if (igMenuItem_Bool("Save", NULL, false, true))
    {
      if (strlen(savePath) == 0)
      {

        const char *path = tinyfd_saveFileDialog("Save As", "default.bud", 1,
                                                 (const char *[]){"*.bud"},
                                                 "Budget File (*.bud)");

        if (path)
        {
          BudgetSave(entryMap, incomeMap, path);
          AddRecentFile(&cfg, path);
          SaveConfig(&cfg);
        }
      }
      else
      {
        BudgetSave(entryMap, incomeMap, savePath);
      }
    }
    if (igMenuItem_Bool("Save As", NULL, false, true))
    {
      const char *path = tinyfd_saveFileDialog("Save As", "default.bud", 1,
                                               (const char *[]){"*.bud"},
                                               "Budget File (*.bud)");

      if (path)
      {
        BudgetSave(entryMap, incomeMap, path);
        AddRecentFile(&cfg, path);
        SaveConfig(&cfg);
      }
    }
    if (igBeginMenu("Recent files", true))
    {
      if (cfg.recentFileCount == 0)
      {
        igMenuItem_Bool("No recent files", NULL, false, false);
      }
      else
      {
        for (int i = 0; i < cfg.recentFileCount; i++)
        {
          const char *fullPath = cfg.recentFiles[i];
          if (fullPath[0] == '\0') continue;

          /* Check existence */
          struct stat st;
          bool exists = stat(fullPath, &st) == 0;

          const char *displayName = TrimHomePath(fullPath);
          if (igMenuItem_Bool(displayName, NULL, false, exists))
          {
            entryMap = BudgetLoad(fullPath);
            AddRecentFile(&cfg, fullPath);
            SaveConfig(&cfg);
          }
        }

        igSeparator();
        if (igMenuItem_Bool("Clear Recent Files", NULL, false, true))
        {
          cfg.recentFileCount = 0;
          memset(cfg.recentFiles, 0, sizeof(cfg.recentFiles));
          SaveConfig(&cfg);
        }
      }
      igEndMenu();
    }

    igSeparator();

    if (igBeginMenu("Export", true))
    {
      if (igMenuItem_Bool("Text File (*.txt)", NULL, false, true))
      {
        const char *path = tinyfd_saveFileDialog("Export As TXT", "budget.txt",
                                                 1, (const char *[]){"*.txt"},
                                                 "Text File (*.txt)");

        if (path)
          ExportAsTXT(entryMap, path);
      }
      if (igMenuItem_Bool("CSV File (*.csv)", NULL, false, true))
      {

        const char *path = tinyfd_saveFileDialog("Export As CSV", "budget.csv",
                                                 1, (const char *[]){"*.csv"},
                                                 "Comma Separated Values (*.csv)");

        if (path)
          ExportAsCSV(entryMap, path);
      }
      if (igMenuItem_Bool("TSV File (*.tsv)", NULL, false, true))
      {

        const char *path = tinyfd_saveFileDialog("Export As TSV", "budget.tsv",
                                                 1, (const char *[]){"*.tsv"},
                                                 "Tab Separated Values (*.tsv)");

        if (path)
          ExportAsTSV(entryMap, path);
      }
      if (igMenuItem_Bool("Excel File (*.xlsx)", NULL, false, true))
      {

        const char *path = tinyfd_saveFileDialog(
            "Export As XLSX", "budget.xlsx", 1,
            (const char *[]){"*.xlsx"}, "Excel Spreadsheet (*.xlsx)");

        if (path)
          ExportAsXLSX(entryMap, path);
      }
      igEndMenu();
    }
    if (igBeginMenu("Default", true))
    {
      if (igMenuItem_Bool("Open default file", NULL, false, true))
      {
        char path[1024];
        snprintf(path, sizeof(path), "%s/default.bud", GetConfigDir());
        entryMap = BudgetLoad(path);
      }
      if (igMenuItem_Bool("Save to default file", NULL, false, true))
      {
        char path[1024];
        snprintf(path, sizeof(path), "%s/default.bud", GetConfigDir());

        BudgetSave(entryMap, incomeMap, path);
        printf("Saved default budget: %s", path);
      }
      if(igMenuItem_Bool("Reset Default Configuration", NULL, false, true)){
        Config cfg = CreateDefaultConfig();
        SaveConfig(&cfg);
      }
      igEndMenu();
    }
    if (igMenuItem_Bool("Exit", NULL, false, true))
    {
      *running = false;
    }
    igEndMenu();
  }
}

void DrawAboutMenu()
{
  if (igBeginMenu("About", true))
  {
    igText("Mojo Budget");
    igText("Version 2.1.0");
    igText("A simple budgeting application.");
    igText("By Ian \"BadMojo\" Vine");
    igEndMenu();
  }
}

/* Premade theme names in display order */
static const char *s_premadeThemes[] = {
  THEME_DARK, THEME_LIGHT, THEME_CLASSIC,
  THEME_CATPPUCCIN, THEME_NORD, THEME_DRACULA,
};
static const int s_premadeCount = 6;

static Theme  *s_customThemes     = NULL;
static int     s_customThemeCount = 0;
static bool    s_themesLoaded     = false;
static bool    s_themeEditorOpen  = false;
static char    s_themeEditorName[THEME_NAME_MAX] = "";

static void ReloadCustomThemes(void)
{
  char themesDir[1024];
  GetThemesDir(themesDir, sizeof(themesDir));
  FreeCustomThemes(s_customThemes);
  s_customThemes     = NULL;
  s_customThemeCount = 0;
  LoadCustomThemes(themesDir, &s_customThemes, &s_customThemeCount);
  s_themesLoaded = true;
}

static void DrawThemeEditorModal(void)
{
  if (s_themeEditorOpen)
  {
    igOpenPopup_Str("Theme Editor##modal", 0);
    s_themeEditorOpen = false;
  }

  ImGuiViewport *vp = igGetMainViewport();
  ImVec2 center = {
    vp->WorkPos.x + vp->WorkSize.x * 0.5f,
    vp->WorkPos.y + vp->WorkSize.y * 0.5f,
  };
  igSetNextWindowPos(center, ImGuiCond_Appearing, (ImVec2){0.5f, 0.5f});
  igSetNextWindowSize((ImVec2){640.0f, 560.0f}, ImGuiCond_Appearing);

  if (igBeginPopupModal("Theme Editor##modal", NULL, ImGuiWindowFlags_None))
  {
    igText("Theme Name:");
    igSameLine(0.0f, 8.0f);
    igSetNextItemWidth(360.0f);
    igInputText("##themename", s_themeEditorName,
                sizeof(s_themeEditorName), 0, NULL, NULL);

    igSeparator();

    /* Scrollable color list */
    igBeginChild_Str("##colorlist",
                     (ImVec2){0.0f, -igGetFrameHeightWithSpacing() * 2.2f},
                     ImGuiChildFlags_Borders, ImGuiWindowFlags_None);

    ImGuiStyle *style = igGetStyle();
    for (int i = 0; i < ImGuiCol_COUNT; i++)
    {
      igPushID_Int(i);
      igColorEdit4(igGetStyleColorName(i), (float *)&style->Colors[i],
                   ImGuiColorEditFlags_AlphaBar |
                   ImGuiColorEditFlags_AlphaPreviewHalf);
      igPopID();
    }

    igEndChild();

    igSeparator();

    if (igButton("Save Theme", (ImVec2){0.0f, 0.0f}))
    {
      if (s_themeEditorName[0] != '\0')
      {
        char themesDir[1024];
        GetThemesDir(themesDir, sizeof(themesDir));
        if (SaveCustomTheme(themesDir, s_themeEditorName))
        {
          ReloadCustomThemes();
          strncpy(cfg.activeTheme, s_themeEditorName,
                  sizeof(cfg.activeTheme) - 1);
          cfg.activeTheme[sizeof(cfg.activeTheme) - 1] = '\0';
          SaveConfig(&cfg);
        }
        igCloseCurrentPopup();
      }
    }
    igSameLine(0.0f, 10.0f);
    if (igButton("Cancel", (ImVec2){0.0f, 0.0f}))
      igCloseCurrentPopup();

    igEndPopup();
  }
}

void DrawThemesMenu(void)
{
  if (!s_themesLoaded)
    ReloadCustomThemes();

  if (igBeginMenu("Themes", true))
  {
    /* Premade themes */
    for (int i = 0; i < s_premadeCount; i++)
    {
      bool selected = strcmp(cfg.activeTheme, s_premadeThemes[i]) == 0;
      if (igMenuItem_Bool(s_premadeThemes[i], NULL, selected, true))
      {
        ApplyPremadeTheme(s_premadeThemes[i]);
        strncpy(cfg.activeTheme, s_premadeThemes[i],
                sizeof(cfg.activeTheme) - 1);
        cfg.activeTheme[sizeof(cfg.activeTheme) - 1] = '\0';
        SaveConfig(&cfg);
      }
    }

    /* Custom themes */
    if (s_customThemeCount > 0)
    {
      igSeparator();
      for (int i = 0; i < s_customThemeCount; i++)
      {
        bool selected = strcmp(cfg.activeTheme, s_customThemes[i].name) == 0;
        if (igMenuItem_Bool(s_customThemes[i].name, NULL, selected, true))
        {
          ApplyCustomTheme(&s_customThemes[i]);
          strncpy(cfg.activeTheme, s_customThemes[i].name,
                  sizeof(cfg.activeTheme) - 1);
          cfg.activeTheme[sizeof(cfg.activeTheme) - 1] = '\0';
          SaveConfig(&cfg);
        }
      }
    }

    igSeparator();

    if (igMenuItem_Bool("New Custom Theme...", NULL, false, true))
    {
      s_themeEditorName[0] = '\0';
      s_themeEditorOpen    = true;
    }
    if (igMenuItem_Bool("Reload Themes", NULL, false, true))
      ReloadCustomThemes();

    igEndMenu();
  }

  DrawThemeEditorModal();
}

void DrawAppMenuBar(bool *running)
{
  if (igBeginMainMenuBar())
  {
    DrawFileMenu(running);

    if (igBeginMenu("View", true))
    {
      if (igMenuItem_Bool("Calendar", NULL, g_showCalendar, true))
        g_showCalendar = !g_showCalendar;
      igEndMenu();
    }

    DrawThemesMenu();
    DrawAboutMenu();
    
    float menuBarWidth = igGetWindowWidth();
    const char* fileName = TrimPath(savePath);
    float textWidth = igCalcTextSize(fileName, NULL, false, 0.0f).x;
    igSetCursorPosX((menuBarWidth - textWidth) / 2.0f);
    igText("%s", fileName);

    igEndMainMenuBar();
  }
}

void DrawUI(bool *running)
{
  DrawAppMenuBar(running);
  DrawBudgetWindow();
  DrawCalendarWindow();
}
