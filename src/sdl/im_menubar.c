#include "im_menubar.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../budget/bill.h"
#include "cimgui.h"
#include <stb/stb_ds.h>

void DrawMenuBar(bool* running)
{
  if (igBeginMainMenuBar())
  {
    if (igBeginMenu("File", true))
    {
      if (igMenuItem_Bool("New", NULL, false, true))
      {
        // Handle New action
      }
      if (igMenuItem_Bool("Open", NULL, false, true))
      {
        // Handle Open action
      }
      if (igMenuItem_Bool("Save", NULL, false, true))
      {
        // Handle Save action
      }
      if (igMenuItem_Bool("Save As", NULL, false, true))
      {
        // Handle Save As action
      }
      if (igBeginMenu("Export", true))
      {
        if (igMenuItem_Bool("Text File (*.txt)", NULL, false, true))
        {
          // Handle Export to TXT action
        }
        igEndMenu();
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

void DrawBudgetWindow()
{
  ImGuiViewport* viewport = igGetMainViewport();
  float menuBarHeight = igGetFrameHeight();
  ImVec2 panelPos = viewport->Pos;
  panelPos.y += menuBarHeight;
  ImVec2 panelSize = viewport->Size;
  panelSize.y -= menuBarHeight;

  igSetNextWindowPos(panelPos, ImGuiCond_Always, (ImVec2) { 0, 0 });
  igSetNextWindowSize(panelSize, ImGuiCond_Always);

  if (igBeginTable("BillsTable", 9,
    ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg,
    (ImVec2) {
    0, 0  
}, 0))
  {
    // Header
    igTableSetupColumn("Name", 0, 0.0f, 0);
    igTableSetupColumn("Frequency", 0, 0.0f, 0);
    igTableSetupColumn("Amount", 0, 0.0f, 0);
    igTableSetupColumn("Weekly", 0, 0.0f, 0);
    igTableSetupColumn("Fortnightly", 0, 0.0f, 0);
    igTableSetupColumn("Monthly", 0, 0.0f, 0);
    igTableSetupColumn("Quarterly", 0, 0.0f, 0);
    igTableSetupColumn("Yearly", 0, 0.0f, 0);
    igTableSetupColumn("Delete", 0, 0.0f, 0);
    igTableHeadersRow();

    int entryCount = hmlen(entryMap);
    for (int i = 0; i < entryCount; i++)
    {
      BillEntry* entry = &entryMap[i];
      Bill* bill = &entry->value;

      igTableNextRow(0, 0);

      // Name (editable)
      igTableSetColumnIndex(0);
      igPushID_Int(entry->key);
      igInputText("##name", bill->name, sizeof(bill->name), 0, NULL, NULL);

      // Frequency (dropdown)
      igTableSetColumnIndex(1);
      const char* freq_names[] = { "Weekly", "Fortnightly", "Monthly",
                                  "Quarterly", "Yearly" };
      int freq = bill->frequency;
      igCombo_Str_arr("##freq", &freq, freq_names, 5, 5);
      bill->frequency = freq;

      // Amount (editable)
      igTableSetColumnIndex(2);
      igInputDouble("##amount", &bill->payment, 0, 0, "%.2f", 0);

      // Calculated columns
      double w = ConvertBillPaymentFrequency(bill, WEEKLY);
      double f = ConvertBillPaymentFrequency(bill, FORTNIGHTLY);
      double m = ConvertBillPaymentFrequency(bill, MONTHLY);
      double q = ConvertBillPaymentFrequency(bill, QUARTERLY);
      double y = ConvertBillPaymentFrequency(bill, YEARLY);

      igTableSetColumnIndex(3);
      igText("$%.2f", w);
      igTableSetColumnIndex(4);
      igText("$%.2f", f);
      igTableSetColumnIndex(5);
      igText("$%.2f", m);
      igTableSetColumnIndex(6);
      igText("$%.2f", q);
      igTableSetColumnIndex(7);
      igText("$%.2f", y);

      // Delete button
      igTableSetColumnIndex(8);
      if (igButton("Delete", (ImVec2) { 0, 0 }))
      {
        RemoveEntry(&entryMap, entry->key);
        // Optionally break and refresh table after deletion
      }
      igPopID();
    }
    igEndTable();
  }
}

void DrawUI(bool* running)
{
  DrawMenuBar(running);
  DrawBudgetWindow();
}
