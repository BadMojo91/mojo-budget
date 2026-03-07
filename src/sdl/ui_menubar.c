#include "ui_core.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "tinyfiledialogs.h"
#include <stb/stb_ds.h>
#include <stdio.h>

#include "../core/bill.h"
#include "../core/config.h"
#include "../core/export.h"
#include "../core/utility.h"

void DrawFileMenu(bool *running)
{
  if (igBeginMenu("File", true))
  {
    if (igMenuItem_Bool("New", NULL, false, true))
    {
      ClearEntries(&entryMap);
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
        entryMap = BudgetLoad(path);
    }
    if (igMenuItem_Bool("Save", NULL, false, true))
    {
      if (strlen(savePath) == 0)
      {

        const char *path = tinyfd_saveFileDialog("Save As", "default.bud", 1,
                                                 (const char *[]){"*.bud"},
                                                 "Budget File (*.bud)");

        if (path)
          BudgetSave(entryMap, path);
      }
      else
      {
        BudgetSave(entryMap, savePath);
      }
    }
    if (igMenuItem_Bool("Save As", NULL, false, true))
    {
      const char *path = tinyfd_saveFileDialog("Save As", "default.bud", 1,
                                               (const char *[]){"*.bud"},
                                               "Budget File (*.bud)");

      if (path)
        BudgetSave(entryMap, path);
    }
    if (igBeginMenu("Recent files", true))
    {
      igText("Not implemented yet");
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

        BudgetSave(entryMap, path);
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
    igText("Version 1.0.0");
    igText("A simple budgeting application built with C and ImGui.");
    igText("By Ian \"BadMojo\" Vine");
    igEndMenu();
  }
}

void DrawMenuBar(bool *running)
{
  if (igBeginMainMenuBar())
  {
    DrawFileMenu(running);
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
  DrawMenuBar(running);
  DrawBudgetWindow();
}
