#include "im_menubar.h"
#include <stdbool.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../budget/bill.h"
#include "../budget/export.h"
#include "cimgui.h"
#include <stdio.h>
#include <stb/stb_ds.h>

void DrawMenuBar(bool* running)
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
        if (igMenuItem_Bool("CSV File (*.csv)", NULL, false, true))
        {
          ExportAsCSV(&entryMap, "exported_bills.csv");
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
  static const char* freq_names[] = {
    "Weekly", "Fortnightly", "Monthly", "Quarterly", "Yearly"
  };

  ImGuiViewport* viewport = igGetMainViewport();
  float menuBarHeight = igGetFrameHeight();
  ImVec2 panelPos = viewport->Pos;
  panelPos.y += menuBarHeight;
  ImVec2 panelSize = viewport->Size;
  panelSize.y -= menuBarHeight;
  double totals[5] = { 0.0 };
  bool removeRequested = false;
  uint64_t removeKey = 0;

  igSetNextWindowPos(panelPos, ImGuiCond_Always, (ImVec2) { 0, 0 });
  igSetNextWindowSize(panelSize, ImGuiCond_Always);

  if (!igBegin("Bills", NULL,
    ImGuiWindowFlags_NoTitleBar |
    ImGuiWindowFlags_NoCollapse |
    ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoResize))
  {
    igEnd();
    return;
  }

  int entryCount = hmlen(entryMap);
  float rowHeight = igGetFrameHeight() + 4.0f;
  float desiredTableHeight = rowHeight * (float)(entryCount + 2);
  ImVec2_c avail = igGetContentRegionAvail();
  float maxTableHeight = avail.y - 120.0f;
  if (maxTableHeight < rowHeight * 4.0f)
  {
    maxTableHeight = rowHeight * 4.0f;
  }
  float tableHeight = desiredTableHeight < maxTableHeight ? desiredTableHeight : maxTableHeight;

  if (igBeginTable("BillsEntries", 9,
    ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
    ImGuiTableFlags_Resizable |
    ImGuiTableFlags_SizingFixedFit |
    ImGuiTableFlags_ScrollX |
    ImGuiTableFlags_ScrollY |
    ImGuiTableFlags_NoKeepColumnsVisible,
    (ImVec2) {
    0, tableHeight
  }, 0))
  {
    igTableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 280.0f, 0);
    igTableSetupColumn("Frequency", ImGuiTableColumnFlags_WidthFixed, 140.0f, 0);
    igTableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed, 120.0f, 0);
    igTableSetupColumn("Weekly", ImGuiTableColumnFlags_WidthFixed, 120.0f, 0);
    igTableSetupColumn("Fortnightly", ImGuiTableColumnFlags_WidthFixed, 120.0f, 0);
    igTableSetupColumn("Monthly", ImGuiTableColumnFlags_WidthFixed, 120.0f, 0);
    igTableSetupColumn("Quarterly", ImGuiTableColumnFlags_WidthFixed, 120.0f, 0);
    igTableSetupColumn("Yearly", ImGuiTableColumnFlags_WidthFixed, 120.0f, 0);
    igTableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoHide, 220.0f, 0);
    igTableHeadersRow();

    for (int i = 0; i < entryCount; i++)
    {
      BillEntry* entry = &entryMap[i];
      Bill* bill = &entry->value;

      igTableNextRow(0, 0);

      // Name
      igTableSetColumnIndex(0);
      igPushID_Int(entry->key);

      if (bill->locked)
      {
        igText("%s", bill->name);
      }
      else
      {
        igSetNextItemWidth(-1.0f);
        igInputText("##name", bill->name, sizeof(bill->name), 0, NULL, NULL);
      }

      // Frequency (dropdown)
      igTableSetColumnIndex(1);
      if (bill->locked)
      {
        igText("%s", freq_names[bill->frequency]);
      }
      else
      {
        int freq = bill->frequency;
        igSetNextItemWidth(-1.0f);
        igCombo_Str_arr("##freq", &freq, freq_names, 5, 5);
        bill->frequency = freq;
      }

      // Amount (editable)
      igTableSetColumnIndex(2);
      if (bill->locked)
      {
        igText("$%.2f", bill->payment);
      }
      else
      {
        igSetNextItemWidth(-1.0f);
        igInputDouble("##amount", &bill->payment, 0, 0, "$%.2f", 0);
      }

      // Calculated columns
      double w = ConvertBillPaymentFrequency(bill, WEEKLY);
      double f = ConvertBillPaymentFrequency(bill, FORTNIGHTLY);
      double m = ConvertBillPaymentFrequency(bill, MONTHLY);
      double q = ConvertBillPaymentFrequency(bill, QUARTERLY);
      double y = ConvertBillPaymentFrequency(bill, YEARLY);

      if (bill->include_in_totals)
      {
        totals[WEEKLY] += w;
        totals[FORTNIGHTLY] += f;
        totals[MONTHLY] += m;
        totals[QUARTERLY] += q;
        totals[YEARLY] += y;
      }

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

      // Row actions
      igTableSetColumnIndex(8);
      const char* useLabel = bill->include_in_totals ? "On" : "Off";
      const char* lockLabel = bill->locked ? "Unlock" : "Lock";
      const float spacing = 4.0f;
      const float buttonPadding = 16.0f;
      float actionsWidth = igGetContentRegionAvail().x;
      float useWidth = igCalcTextSize(useLabel, NULL, false, -1.0f).x + buttonPadding;
      float lockWidth = igCalcTextSize(lockLabel, NULL, false, -1.0f).x + buttonPadding;
      float deleteWidth = igCalcTextSize("Delete", NULL, false, -1.0f).x + buttonPadding;

      if (igSmallButton(useLabel))
      {
        bill->include_in_totals = !bill->include_in_totals;
      }

      if (actionsWidth >= (useWidth + spacing + lockWidth + spacing + deleteWidth))
      {
        igSameLine(0.0f, spacing);
      }

      if (igSmallButton(lockLabel))
      {
        bill->locked = !bill->locked;
      }

      if (actionsWidth >= (useWidth + spacing + lockWidth + spacing + deleteWidth) ||
        actionsWidth >= (lockWidth + spacing + deleteWidth))
      {
        igSameLine(0.0f, spacing);
      }

      if (igSmallButton("Delete"))
      {
        removeRequested = true;
        removeKey = entry->key;
      }
      igPopID();

      if (removeRequested)
      {
        break;
      }
    }

    // Add default entry row
    igTableNextRow(0, 0.0f);
    igTableSetColumnIndex(0);
    igText("Add default entry");

    igTableSetColumnIndex(8);
    if (igSmallButton("+ Add"))
    {
      Bill defaultBill = { 0 };
      snprintf(defaultBill.name, sizeof(defaultBill.name), "New Bill");
      defaultBill.frequency = MONTHLY;
      defaultBill.payment = 0.0;
      defaultBill.include_in_totals = true;
      defaultBill.locked = false;
      AddEntry(&entryMap, defaultBill);
    }

    igEndTable();
  }

  if (removeRequested)
  {
    RemoveEntry(&entryMap, removeKey);
  }

  igSeparator();
  igText("Totals (enabled bills)");
  igText("Weekly: $%.2f", totals[WEEKLY]);
  igText("Fortnightly: $%.2f", totals[FORTNIGHTLY]);
  igText("Monthly: $%.2f", totals[MONTHLY]);
  igText("Quarterly: $%.2f", totals[QUARTERLY]);
  igText("Yearly: $%.2f", totals[YEARLY]);

  igEnd();
}

void DrawUI(bool* running)
{
  DrawMenuBar(running);
  DrawBudgetWindow();
}
