#include "ui_core.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../core/bill.h"
#include "cimgui.h"
#include <stb/stb_ds.h>
#include <math.h>
#include <stdio.h>

#include "../core/config.h"

extern Config cfg;

static const char* g_freqNames[] = { "Weekly", "Fortnightly", "Monthly",
  "Quarterly", "Yearly" };

typedef struct
{
  float tableHeight;
  float minTableHeight;
  float maxTableHeight;
} BillTableLayout;

static void ConfigureBudgetWindowLayout()
{
  ImGuiViewport* viewport = igGetMainViewport();
  float menuBarHeight = igGetFrameHeight();
  ImVec2 panelPos = viewport->Pos;
  panelPos.y += menuBarHeight;
  ImVec2 panelSize = viewport->Size;
  panelSize.y -= menuBarHeight;

  igSetNextWindowPos(panelPos, ImGuiCond_Always, (ImVec2) { 0, 0 });
  igSetNextWindowSize(panelSize, ImGuiCond_Always);
}

static BillTableLayout ComputeBillTableLayout(int entryCount)
{
  BillTableLayout layout = { 0 };
  float rowHeight = igGetFrameHeight() + 4.0f;
  float desiredTableHeight = rowHeight * (float)(entryCount + 2);
  ImVec2_c avail = igGetContentRegionAvail();

  layout.maxTableHeight = avail.y - 120.0f;
  if (layout.maxTableHeight < rowHeight * 4.0f)
  {
    layout.maxTableHeight = rowHeight * 4.0f;
  }

  layout.minTableHeight = rowHeight * 4.0f;
  float defaultTableHeight = desiredTableHeight < layout.maxTableHeight
    ? desiredTableHeight
    : layout.maxTableHeight;

  layout.tableHeight =
    cfg.bill_table_height > 0.0f ? cfg.bill_table_height : defaultTableHeight;
  if (layout.tableHeight < layout.minTableHeight)
  {
    layout.tableHeight = layout.minTableHeight;
  }
  if (layout.tableHeight > layout.maxTableHeight)
  {
    layout.tableHeight = layout.maxTableHeight;
  }

  return layout;
}

static void SetupBillsTableColumns()
{
  float* widths = cfg.bill_column_widths;
  igTableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, widths[0], 0);
  igTableSetupColumn("Frequency", ImGuiTableColumnFlags_WidthFixed, widths[1],
    0);
  igTableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed, widths[2], 0);
  igTableSetupColumn("Weekly", ImGuiTableColumnFlags_WidthFixed, widths[3], 0);
  igTableSetupColumn("Fortnightly", ImGuiTableColumnFlags_WidthFixed,
    widths[4], 0);
  igTableSetupColumn("Monthly", ImGuiTableColumnFlags_WidthFixed, widths[5],
    0);
  igTableSetupColumn("Quarterly", ImGuiTableColumnFlags_WidthFixed, widths[6],
    0);
  igTableSetupColumn("Yearly", ImGuiTableColumnFlags_WidthFixed, widths[7], 0);
  igTableSetupColumn("Parameters",
    ImGuiTableColumnFlags_WidthFixed |
    ImGuiTableColumnFlags_NoResize |
    ImGuiTableColumnFlags_NoHide,
    164, 0);
  igTableHeadersRow();
}

static void DrawBillNameColumn(Bill* bill)
{
  igTableSetColumnIndex(0);
  if (bill->locked)
  {
    igText("%s", bill->name);
  }
  else
  {
    igSetNextItemWidth(-1.0f);
    igInputText("##name", bill->name, sizeof(bill->name), 0, NULL, NULL);
  }
}

static void DrawBillFrequencyColumn(Bill* bill)
{
  igTableSetColumnIndex(1);
  if (bill->locked)
  {
    igText("%s", g_freqNames[bill->frequency]);
  }
  else
  {
    int frequency = bill->frequency;
    igSetNextItemWidth(-1.0f);
    igCombo_Str_arr("##freq", &frequency, g_freqNames, 5, 5);
    bill->frequency = frequency;
  }
}

static void DrawBillPaymentColumn(Bill* bill)
{
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
}

static void GetBillConvertedAmounts(Bill* bill, double amounts[5])
{
  amounts[WEEKLY] = ConvertBillPaymentFrequency(bill, WEEKLY);
  amounts[FORTNIGHTLY] = ConvertBillPaymentFrequency(bill, FORTNIGHTLY);
  amounts[MONTHLY] = ConvertBillPaymentFrequency(bill, MONTHLY);
  amounts[QUARTERLY] = ConvertBillPaymentFrequency(bill, QUARTERLY);
  amounts[YEARLY] = ConvertBillPaymentFrequency(bill, YEARLY);
}

static void AccumulateBillTotals(Bill* bill, const double amounts[5],
  double totals[5])
{
  if (!bill->include_in_totals)
  {
    return;
  }

  totals[WEEKLY] += amounts[WEEKLY];
  totals[FORTNIGHTLY] += amounts[FORTNIGHTLY];
  totals[MONTHLY] += amounts[MONTHLY];
  totals[QUARTERLY] += amounts[QUARTERLY];
  totals[YEARLY] += amounts[YEARLY];
}

static void DrawBillConvertedColumns(const double amounts[5])
{
  igTableSetColumnIndex(3);
  igText("$%.2f", amounts[WEEKLY]);
  igTableSetColumnIndex(4);
  igText("$%.2f", amounts[FORTNIGHTLY]);
  igTableSetColumnIndex(5);
  igText("$%.2f", amounts[MONTHLY]);
  igTableSetColumnIndex(6);
  igText("$%.2f", amounts[QUARTERLY]);
  igTableSetColumnIndex(7);
  igText("$%.2f", amounts[YEARLY]);
}

static void DrawBillActions(BillEntry* entry, bool* removeRequested,
  uint64_t* removeKey)
{
  Bill* bill = &entry->value;
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
    *removeRequested = true;
    *removeKey = entry->key;
  }
}

static bool DrawBillRow(BillEntry* entry, double totals[5],
  bool* removeRequested, uint64_t* removeKey)
{
  Bill* bill = &entry->value;
  double amounts[5] = { 0.0 };

  igTableNextRow(0, 0);
  igPushID_Int(entry->key);

  DrawBillNameColumn(bill);
  DrawBillFrequencyColumn(bill);
  DrawBillPaymentColumn(bill);

  GetBillConvertedAmounts(bill, amounts);
  AccumulateBillTotals(bill, amounts, totals);
  DrawBillConvertedColumns(amounts);
  DrawBillActions(entry, removeRequested, removeKey);

  igPopID();
  return *removeRequested;
}

static void PersistBillColumnWidths()
{
  for (int i = 0; i < NUM_BILL_COLUMNS; i++)
  {
    float currentWidth = igGetColumnWidth(i);
    if (currentWidth > 0.0f &&
      fabsf(cfg.bill_column_widths[i] - currentWidth) > 0.5f)
    {
      cfg.bill_column_widths[i] = currentWidth;
    }
  }
}

static void DrawBillTableResizeGrip(const BillTableLayout* layout)
{
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
        cfg.bill_table_height = layout->tableHeight;
      }
      cfg.bill_table_height += delta;
      if (cfg.bill_table_height < layout->minTableHeight)
      {
        cfg.bill_table_height = layout->minTableHeight;
      }
      if (cfg.bill_table_height > layout->maxTableHeight)
      {
        cfg.bill_table_height = layout->maxTableHeight;
      }
    }
  }
  igPopID();
}

static void DrawAddBillButton()
{
  ImVec2 avail = igGetContentRegionAvail();
  float buttonWidth = 80.0f;
  igSetCursorPosX(igGetCursorPosX() + avail.x - buttonWidth);
  if (!igButton("Add", (ImVec2) { buttonWidth, 0 }))
  {
    return;
  }

  Bill defaultBill = { 0 };
  snprintf(defaultBill.name, sizeof(defaultBill.name), "New Bill");
  defaultBill.frequency = MONTHLY;
  defaultBill.payment = 0.0;
  defaultBill.include_in_totals = true;
  defaultBill.locked = false;
  AddEntry(&entryMap, defaultBill);
}

static void DrawBillsTable(const BillTableLayout* layout, double totals[5],
  bool* removeRequested, uint64_t* removeKey)
{
  if (!igBeginTable("BillsEntries", 9,
    ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
    ImGuiTableFlags_Resizable |
    ImGuiTableFlags_SizingFixedFit |
    ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
    ImGuiTableFlags_NoKeepColumnsVisible,
    (ImVec2) {
    0, layout->tableHeight
  }, 0))
  {
    return;
  }

  SetupBillsTableColumns();

  int entryCount = hmlen(entryMap);
  for (int i = 0; i < entryCount; i++)
  {
    if (DrawBillRow(&entryMap[i], totals, removeRequested, removeKey))
    {
      break;
    }
  }

  PersistBillColumnWidths();
  igEndTable();

  DrawBillTableResizeGrip(layout);
  DrawAddBillButton();
}

static void DrawBillTotals(const double totals[5])
{
  igSeparator();
  igText("Totals (enabled bills)");
  igText("Weekly: $%.2f", totals[WEEKLY]);
  igText("Fortnightly: $%.2f", totals[FORTNIGHTLY]);
  igText("Monthly: $%.2f", totals[MONTHLY]);
  igText("Quarterly: $%.2f", totals[QUARTERLY]);
  igText("Yearly: $%.2f", totals[YEARLY]);
}

void DrawBudgetWindow()
{
  double totals[5] = { 0.0 };
  bool removeRequested = false;
  uint64_t removeKey = 0;

  ConfigureBudgetWindowLayout();

  if (!igBegin("Bills", NULL,
    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
  {
    igEnd();
    return;
  }

  int entryCount = hmlen(entryMap);
  BillTableLayout layout = ComputeBillTableLayout(entryCount);
  DrawBillsTable(&layout, totals, &removeRequested, &removeKey);

  if (removeRequested)
  {
    RemoveEntry(&entryMap, removeKey);
  }

  DrawBillTotals(totals);

  igEnd();
}

