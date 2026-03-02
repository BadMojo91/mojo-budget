#include "ui_core.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../core/bill.h"
#include "cimgui.h"
#include <stb/stb_ds.h>
#include <math.h>
#include <stdio.h>

#include "../core/config.h"

extern Config cfg;

void DrawBudgetWindow()
{
  static const char* freq_names[] = { "Weekly", "Fortnightly", "Monthly",
                                     "Quarterly", "Yearly" };

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
    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
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
  float minTableHeight = rowHeight * 4.0f;
  float defaultTableHeight =
    desiredTableHeight < maxTableHeight ? desiredTableHeight : maxTableHeight;
  float tableHeight = cfg.bill_table_height > 0.0f ? cfg.bill_table_height
    : defaultTableHeight;
  if (tableHeight < minTableHeight)
  {
    tableHeight = minTableHeight;
  }
  if (tableHeight > maxTableHeight)
  {
    tableHeight = maxTableHeight;
  }

  if (igBeginTable("BillsEntries", 9,
    ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
    ImGuiTableFlags_Resizable |
    ImGuiTableFlags_SizingFixedFit |
    ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
    ImGuiTableFlags_NoKeepColumnsVisible,
    (ImVec2) {
    0, tableHeight
  }, 0))
  {
    float* w = cfg.bill_column_widths;
    igTableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, w[0], 0);
    igTableSetupColumn("Frequency", ImGuiTableColumnFlags_WidthFixed, w[1], 0);
    igTableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed, w[2], 0);
    igTableSetupColumn("Weekly", ImGuiTableColumnFlags_WidthFixed, w[3], 0);
    igTableSetupColumn("Fortnightly", ImGuiTableColumnFlags_WidthFixed, w[4], 0);
    igTableSetupColumn("Monthly", ImGuiTableColumnFlags_WidthFixed, w[5], 0);
    igTableSetupColumn("Quarterly", ImGuiTableColumnFlags_WidthFixed, w[6], 0);
    igTableSetupColumn("Yearly", ImGuiTableColumnFlags_WidthFixed, w[7], 0);
    igTableSetupColumn("Parameters",
      ImGuiTableColumnFlags_WidthFixed |
      ImGuiTableColumnFlags_NoResize |
      ImGuiTableColumnFlags_NoHide,
      164, 0);
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
      float useWidth =
        igCalcTextSize(useLabel, NULL, false, -1.0f).x + buttonPadding;
      float lockWidth =
        igCalcTextSize(lockLabel, NULL, false, -1.0f).x + buttonPadding;
      float deleteWidth =
        igCalcTextSize("Delete", NULL, false, -1.0f).x + buttonPadding;

      if (igSmallButton(useLabel))
      {
        bill->include_in_totals = !bill->include_in_totals;
      }

      if (actionsWidth >=
        (useWidth + spacing + lockWidth + spacing + deleteWidth))
      {
        igSameLine(0.0f, spacing);
      }

      if (igSmallButton(lockLabel))
      {
        bill->locked = !bill->locked;
      }

      if (actionsWidth >=
        (useWidth + spacing + lockWidth + spacing + deleteWidth) ||
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

    bool columnWidthsChanged = false;
    for (int i = 0; i < NUM_BILL_COLUMNS; i++)
    {
      float currentWidth = igGetColumnWidth(i);
      if (currentWidth > 0.0f &&
        fabsf(cfg.bill_column_widths[i] - currentWidth) > 0.5f)
      {
        cfg.bill_column_widths[i] = currentWidth;
        columnWidthsChanged = true;
      }
    }

    igEndTable();

    ImVec2_c gripAvail = igGetContentRegionAvail();
    float gripHeight = 8.0f;
    float gripWidth = gripAvail.x;
    if (gripWidth < 1.0f)
    {
      gripWidth = 1.0f;
    }

    igPushID_Str("BillTableResizeGrip");
    igInvisibleButton("##resize", (ImVec2) { gripWidth, gripHeight }, 0);
    bool resizeHovered = igIsItemHovered(ImGuiHoveredFlags_None);
    bool resizeActive = igIsItemActive();

    if (resizeHovered || resizeActive)
    {
      igSetMouseCursor(ImGuiMouseCursor_ResizeNS);
    }

    if (resizeActive)
    {
      float delta = igGetIO_Nil()->MouseDelta.y;
      if (delta != 0.0f)
      {
        if (cfg.bill_table_height <= 0.0f)
        {
          cfg.bill_table_height = tableHeight;
        }
        cfg.bill_table_height += delta;
        if (cfg.bill_table_height < minTableHeight)
        {
          cfg.bill_table_height = minTableHeight;
        }
        if (cfg.bill_table_height > maxTableHeight)
        {
          cfg.bill_table_height = maxTableHeight;
        }
      }
    }
    igPopID();

    ImVec2 avail = igGetContentRegionAvail();
    float buttonWidth = 80.0f;
    igSetCursorPosX(igGetCursorPosX() + avail.x - buttonWidth);
    if (igButton("Add", (ImVec2) { buttonWidth, 0 }))
    {
      Bill defaultBill = { 0 };
      snprintf(defaultBill.name, sizeof(defaultBill.name), "New Bill");
      defaultBill.frequency = MONTHLY;
      defaultBill.payment = 0.0;
      defaultBill.include_in_totals = true;
      defaultBill.locked = false;
      AddEntry(&entryMap, defaultBill);
    }
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

