#include "ui_core.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "tinyfiledialogs.h"
#include <stb/stb_ds.h>
#include <stdio.h>

#include "../core/bill.h"
#include "../core/export.h"
#include "../core/config.h"


void DrawMenuBar(bool *running)
{
  if (igBeginMainMenuBar())
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
                                  "Budget Entry Maps", // filter description
                                  0                    // single select
            );
        if (path)
          entryMap = LoadEntryMap(path);
      }
      if (igMenuItem_Bool("Save", NULL, false, true))
      {
        const char *path = tinyfd_saveFileDialog("Save Budget", "default.bud",
                                                 1, (const char *[]){"*.bud"},
                                                 "Budget File (*.bud)");

        if (path)
          SaveEntryMap(path, entryMap, FILETYPE_BUD);
      }
      if (igMenuItem_Bool("Save As", NULL, false, true))
      {
        const char *path = tinyfd_saveFileDialog("Save Budget", "default.bud",
                                                 1, (const char *[]){"*.bud"},
                                                 "Budget File (*.bud)");

        if (path)
          SaveEntryMap(path, entryMap, FILETYPE_BUD);
      }
      if (igBeginMenu("Export", true))
      {
        if (igMenuItem_Bool("Text File (*.txt)", NULL, false, true))
        {
          const char *path = tinyfd_saveFileDialog(
              "Export As TXT", "budget.txt", 1, (const char *[]){"*.txt"},
              "Text File (*.txt)");

          if (path)
            SaveEntryMap(path, entryMap, FILETYPE_TXT);
        }
        if (igMenuItem_Bool("CSV File (*.csv)", NULL, false, true))
        {

          const char *path = tinyfd_saveFileDialog(
              "Export As CSV", "budget.csv", 1, (const char *[]){"*.csv"},
              "CSV File (*.csv)");

          if (path)
            ExportAsCSV(&entryMap, path);
        }
        igEndMenu();
      }
      if (igMenuItem_Bool("Save to default file", NULL, false, true))
      {
        char path[1024];
        snprintf(path, sizeof(path), "%s/default.bud",
                 GetConfigDir());
        
          SaveEntryMap(path, entryMap, FILETYPE_BUD);
        printf("Saved default budget: %s", path);
      }

      if (igMenuItem_Bool("Exit", NULL, false, true))
      {
        *running = false;
      }
      igEndMenu();
    }
    igEndMainMenuBar();
  }
}

void DrawUI(bool *running)
{
  DrawMenuBar(running);
  DrawBudgetWindow();
}
