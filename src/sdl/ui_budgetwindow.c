#include "ui_core.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../core/bill.h"
#include "cimgui.h"
#include <stb/stb_ds.h>
#include <stdio.h>
#include <stdlib.h>

static const char *g_freqNames[] = {"Weekly", "Fortnightly", "Monthly",
                                    "Quarterly", "Yearly"};
static const char *g_incomeFreqNames[] = {"Weekly", "Fortnightly", "Monthly",
                                          "Alternating Weekly"};
static float g_billTableHeight = 0.0f;
static float g_incomeTableHeight = 0.0f;

typedef struct
{
  float tableHeight;
  float minTableHeight;
  float maxTableHeight;
} TableLayout;

// ─── Income table ────────────────────────────────────────────────────────────

static TableLayout ComputeIncomeTableLayout(int entryCount)
{
  TableLayout layout = {0};
  float rowHeight = igGetFrameHeight() + 4.0f;
  float desiredTableHeight = rowHeight * (float)(entryCount + 2);
  ImVec2_c avail = igGetContentRegionAvail();

  layout.maxTableHeight = avail.y * 0.4f;
  if (layout.maxTableHeight < rowHeight * 3.0f)
    layout.maxTableHeight = rowHeight * 3.0f;

  layout.minTableHeight = rowHeight * 3.0f;
  float defaultTableHeight = desiredTableHeight < layout.maxTableHeight
                                 ? desiredTableHeight
                                 : layout.maxTableHeight;

  layout.tableHeight = g_incomeTableHeight > 0.0f ? g_incomeTableHeight
                                                  : defaultTableHeight;
  if (layout.tableHeight < layout.minTableHeight)
    layout.tableHeight = layout.minTableHeight;
  if (layout.tableHeight > layout.maxTableHeight)
    layout.tableHeight = layout.maxTableHeight;

  return layout;
}

static void SetupIncomeTableColumns()
{
  igTableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 105.0f, 0);
  igTableSetupColumn("Frequency", ImGuiTableColumnFlags_WidthFixed, 140.0f, 0);
  igTableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed, 160.0f, 0);
  igTableSetupColumn("Weekly", ImGuiTableColumnFlags_WidthFixed, 70.0f, 0);
  igTableSetupColumn("Fortnightly", ImGuiTableColumnFlags_WidthFixed, 80.0f, 0);
  igTableSetupColumn("Monthly", ImGuiTableColumnFlags_WidthFixed, 75.0f, 0);
  igTableSetupColumn("Quarterly", ImGuiTableColumnFlags_WidthFixed, 80.0f, 0);
  igTableSetupColumn("Yearly", ImGuiTableColumnFlags_WidthFixed, 80.0f, 0);
  igTableSetupColumn("Actions",
                     ImGuiTableColumnFlags_WidthFixed |
                         ImGuiTableColumnFlags_NoResize |
                         ImGuiTableColumnFlags_NoHide,
                     160.0f, 0);
  igTableHeadersRow();
}

static void DrawIncomeNameColumn(Income *income)
{
  igTableSetColumnIndex(0);
  if (income->locked)
  {
    igText("%s", income->name);
  }
  else
  {
    igSetNextItemWidth(-1.0f);
    igInputText("##iname", income->name, sizeof(income->name), 0, NULL, NULL);
  }
}

static void DrawIncomeFrequencyColumn(Income *income)
{
  igTableSetColumnIndex(1);
  if (income->locked)
  {
    igText("%s", g_incomeFreqNames[income->frequency]);
  }
  else
  {
    int freq = (int)income->frequency;
    igSetNextItemWidth(-1.0f);
    igCombo_Str_arr("##ifreq", &freq, g_incomeFreqNames, 4, 4);
    income->frequency = (IncomeFrequency)freq;
  }
}

static void DrawIncomeAmountColumn(Income *income)
{
  igTableSetColumnIndex(2);
  if (income->locked)
  {
    if (income->frequency == INCOME_ALTERNATING_WEEKLY)
    {
      const char *a1 = ConvertDoubleToString(income->amount);
      const char *a2 = ConvertDoubleToString(income->amount_alt);
      igText("W1:$%s  W2:$%s", a1, a2);
      free((void *)a1);
      free((void *)a2);
    }
    else
    {
      const char *s = ConvertDoubleToString(income->amount);
      igText("$%s", s);
      free((void *)s);
    }
  }
  else if (income->frequency == INCOME_ALTERNATING_WEEKLY)
  {
    float halfW = (igGetContentRegionAvail().x - 4.0f) * 0.5f;
    igSetNextItemWidth(halfW);
    igInputDouble("##iamtW1", &income->amount, 0, 0, "W1: $%.2f", 0);
    igSameLine(0.0f, 4.0f);
    igSetNextItemWidth(halfW);
    igInputDouble("##iamtW2", &income->amount_alt, 0, 0, "W2: $%.2f", 0);
  }
  else
  {
    igSetNextItemWidth(-1.0f);
    igInputDouble("##iamt", &income->amount, 0, 0, "$%.2f", 0);
  }
}

static void DrawIncomeConvertedColumns(double weekly, double fortnightly,
                                       double monthly, double quarterly,
                                       double yearly)
{
  const char *s;

  igTableSetColumnIndex(3);
  s = ConvertDoubleToString(weekly);
  igText("$%s", s);
  free((void *)s);

  igTableSetColumnIndex(4);
  s = ConvertDoubleToString(fortnightly);
  igText("$%s", s);
  free((void *)s);

  igTableSetColumnIndex(5);
  s = ConvertDoubleToString(monthly);
  igText("$%s", s);
  free((void *)s);

  igTableSetColumnIndex(6);
  s = ConvertDoubleToString(quarterly);
  igText("$%s", s);
  free((void *)s);

  igTableSetColumnIndex(7);
  s = ConvertDoubleToString(yearly);
  igText("$%s", s);
  free((void *)s);
}

static void DrawIncomeActions(IncomeEntry *entry, bool *removeRequested,
                              uint64_t *removeKey)
{
  Income *income = &entry->value;
  igTableSetColumnIndex(8);

  const char *enableLabel = income->enable ? "On" : "Off";
  const char *lockLabel = income->locked ? "Unlock" : "Lock";
  const float spacing = 4.0f;
  const float buttonPadding = 16.0f;
  float enableWidth =
      igCalcTextSize(enableLabel, NULL, false, -1.0f).x + buttonPadding;
  float lockWidth =
      igCalcTextSize(lockLabel, NULL, false, -1.0f).x + buttonPadding;
  float deleteWidth =
      igCalcTextSize("Delete", NULL, false, -1.0f).x + buttonPadding;
  float actionsWidth = igGetContentRegionAvail().x;

  if (igSmallButton(enableLabel))
    income->enable = !income->enable;

  if (actionsWidth >= (enableWidth + spacing + lockWidth))
    igSameLine(0.0f, spacing);

  if (igSmallButton(lockLabel))
    income->locked = !income->locked;

  if (actionsWidth >= (enableWidth + spacing + lockWidth + spacing + deleteWidth))
    igSameLine(0.0f, spacing);

  if (igSmallButton("Delete"))
  {
    *removeRequested = true;
    *removeKey = entry->key;
  }
}

static bool DrawIncomeRow(IncomeEntry *entry, double totals[5],
                          bool *removeRequested, uint64_t *removeKey)
{
  Income *income = &entry->value;

  igTableNextRow(0, 0);
  igPushID_Int((int)entry->key + 10000); // offset to avoid colliding with bill IDs

  DrawIncomeNameColumn(income);
  DrawIncomeFrequencyColumn(income);
  DrawIncomeAmountColumn(income);

  double w = GetIncomeAtFrequency(income, INCOME_WEEKLY);
  double f = GetIncomeAtFrequency(income, INCOME_FORTNIGHTLY);
  double m = GetIncomeAtFrequency(income, INCOME_MONTHLY);
  double q = w * 13.0;
  double y = w * 52.0;

  if (income->enable)
  {
    totals[0] += w;
    totals[1] += f;
    totals[2] += m;
    totals[3] += q;
    totals[4] += y;
  }

  DrawIncomeConvertedColumns(w, f, m, q, y);
  DrawIncomeActions(entry, removeRequested, removeKey);

  igPopID();
  return *removeRequested;
}

static void DrawIncomeTableResizeGrip(const TableLayout *layout)
{
  ImVec2_c gripAvail = igGetContentRegionAvail();
  float gripHeight = 8.0f;
  float gripWidth = gripAvail.x > 1.0f ? gripAvail.x : 1.0f;

  igPushID_Str("IncomeTableResizeGrip");
  igInvisibleButton("##iresize", (ImVec2){gripWidth, gripHeight}, 0);
  bool resizeHovered = igIsItemHovered(ImGuiHoveredFlags_None);
  bool resizeActive = igIsItemActive();

  if (resizeHovered || resizeActive)
    igSetMouseCursor(ImGuiMouseCursor_ResizeNS);

  if (resizeActive)
  {
    float delta = igGetIO_Nil()->MouseDelta.y;
    if (delta != 0.0f)
    {
      if (g_incomeTableHeight <= 0.0f)
        g_incomeTableHeight = layout->tableHeight;
      g_incomeTableHeight += delta;
      if (g_incomeTableHeight < layout->minTableHeight)
        g_incomeTableHeight = layout->minTableHeight;
      if (g_incomeTableHeight > layout->maxTableHeight)
        g_incomeTableHeight = layout->maxTableHeight;
    }
  }
  igPopID();
}

static void DrawAddIncomeButton()
{
  ImVec2 avail = igGetContentRegionAvail();
  float buttonWidth = 80.0f;
  igSetCursorPosX(igGetCursorPosX() + avail.x - buttonWidth);
  if (!igButton("Add##income", (ImVec2){buttonWidth, 0}))
    return;

  Income defaultIncome = {0};
  snprintf(defaultIncome.name, sizeof(defaultIncome.name), "New Income");
  defaultIncome.frequency = INCOME_MONTHLY;
  defaultIncome.amount = 0.0;
  defaultIncome.amount_alt = 0.0;
  defaultIncome.enable = true;
  AddIncomeEntry(&incomeMap, defaultIncome);
}

static void DrawIncomeTable(double totals[5], bool *removeRequested,
                            uint64_t *removeKey)
{
  int entryCount = hmlen(incomeMap);
  TableLayout layout = ComputeIncomeTableLayout(entryCount);

  if (!igBeginTable("IncomeEntries", 9,
                    ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                        ImGuiTableFlags_Resizable |
                        ImGuiTableFlags_SizingFixedFit |
                        ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
                        ImGuiTableFlags_NoKeepColumnsVisible,
                    (ImVec2){0, layout.tableHeight}, 0))
  {
    return;
  }

  SetupIncomeTableColumns();

  for (int i = 0; i < entryCount; i++)
  {
    if (DrawIncomeRow(&incomeMap[i], totals, removeRequested, removeKey))
      break;
  }

  igEndTable();

  DrawIncomeTableResizeGrip(&layout);
  DrawAddIncomeButton();
}

// ─── Bills table ─────────────────────────────────────────────────────────────

static void ConfigureBudgetWindowLayout()
{
  ImGuiViewport *viewport = igGetMainViewport();
  float menuBarHeight = igGetFrameHeight();
  ImVec2 panelPos = viewport->Pos;
  panelPos.y += menuBarHeight;
  ImVec2 panelSize = viewport->Size;
  panelSize.y -= menuBarHeight;

  igSetNextWindowPos(panelPos, ImGuiCond_Always, (ImVec2){0, 0});
  igSetNextWindowSize(panelSize, ImGuiCond_Always);
}

static TableLayout ComputeBillTableLayout(int entryCount)
{
  TableLayout layout = {0};
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

  layout.tableHeight = g_billTableHeight > 0.0f ? g_billTableHeight
                                                : defaultTableHeight;
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
  igTableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 105.0f, 0);
  igTableSetupColumn("Frequency", ImGuiTableColumnFlags_WidthFixed, 102.0f, 0);
  igTableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed, 64.0f, 0);
  igTableSetupColumn("Weekly", ImGuiTableColumnFlags_WidthFixed, 64.0f, 0);
  igTableSetupColumn("Fortnightly", ImGuiTableColumnFlags_WidthFixed, 75.0f,
                     0);
  igTableSetupColumn("Monthly", ImGuiTableColumnFlags_WidthFixed, 75.0f, 0);
  igTableSetupColumn("Quarterly", ImGuiTableColumnFlags_WidthFixed, 75.0f, 0);
  igTableSetupColumn("Yearly", ImGuiTableColumnFlags_WidthFixed, 75.0f, 0);
  igTableSetupColumn("Parameters",
                     ImGuiTableColumnFlags_WidthFixed |
                         ImGuiTableColumnFlags_NoResize |
                         ImGuiTableColumnFlags_NoHide,
                     164, 0);
  igTableHeadersRow();
}

static void DrawBillNameColumn(Bill *bill)
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

static void DrawBillFrequencyColumn(Bill *bill)
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

static void DrawBillPaymentColumn(Bill *bill)
{
  igTableSetColumnIndex(2);
  if (bill->locked)
  {
    const char *s = ConvertDoubleToString(bill->payment);
    igText("$%s", s);
    free((void *)s);
  }
  else
  {
    igSetNextItemWidth(-1.0f);
    igInputDouble("##amount", &bill->payment, 0, 0, "$%.2f", 0);
  }
}

static void GetBillConvertedAmounts(Bill *bill, double amounts[5])
{
  amounts[WEEKLY] = ConvertBillPaymentFrequency(bill, WEEKLY);
  amounts[FORTNIGHTLY] = ConvertBillPaymentFrequency(bill, FORTNIGHTLY);
  amounts[MONTHLY] = ConvertBillPaymentFrequency(bill, MONTHLY);
  amounts[QUARTERLY] = ConvertBillPaymentFrequency(bill, QUARTERLY);
  amounts[YEARLY] = ConvertBillPaymentFrequency(bill, YEARLY);
}

static void AccumulateBillTotals(Bill *bill, const double amounts[5],
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
  const char *s;

  igTableSetColumnIndex(3);
  s = ConvertDoubleToString(amounts[WEEKLY]);
  igText("$%s", s);
  free((void *)s);

  igTableSetColumnIndex(4);
  s = ConvertDoubleToString(amounts[FORTNIGHTLY]);
  igText("$%s", s);
  free((void *)s);

  igTableSetColumnIndex(5);
  s = ConvertDoubleToString(amounts[MONTHLY]);
  igText("$%s", s);
  free((void *)s);

  igTableSetColumnIndex(6);
  s = ConvertDoubleToString(amounts[QUARTERLY]);
  igText("$%s", s);
  free((void *)s);

  igTableSetColumnIndex(7);
  s = ConvertDoubleToString(amounts[YEARLY]);
  igText("$%s", s);
  free((void *)s);
}

static void DrawBillActions(BillEntry *entry, bool *removeRequested,
                            uint64_t *removeKey)
{
  Bill *bill = &entry->value;
  igTableSetColumnIndex(8);

  const char *useLabel = bill->include_in_totals ? "On" : "Off";
  const char *lockLabel = bill->locked ? "Unlock" : "Lock";
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

  if (actionsWidth >=
          (useWidth + spacing + lockWidth + spacing + deleteWidth) ||
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

static bool DrawBillRow(BillEntry *entry, double totals[5],
                        bool *removeRequested, uint64_t *removeKey)
{
  Bill *bill = &entry->value;
  double amounts[5] = {0.0};

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

static void DrawBillTableResizeGrip(const TableLayout *layout)
{
  ImVec2_c gripAvail = igGetContentRegionAvail();
  float gripHeight = 8.0f;
  float gripWidth = gripAvail.x;
  if (gripWidth < 1.0f)
  {
    gripWidth = 1.0f;
  }

  igPushID_Str("BillTableResizeGrip");
  igInvisibleButton("##resize", (ImVec2){gripWidth, gripHeight}, 0);
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
      if (g_billTableHeight <= 0.0f)
      {
        g_billTableHeight = layout->tableHeight;
      }
      g_billTableHeight += delta;
      if (g_billTableHeight < layout->minTableHeight)
      {
        g_billTableHeight = layout->minTableHeight;
      }
      if (g_billTableHeight > layout->maxTableHeight)
      {
        g_billTableHeight = layout->maxTableHeight;
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
  if (!igButton("Add", (ImVec2){buttonWidth, 0}))
  {
    return;
  }

  Bill defaultBill = {0};
  snprintf(defaultBill.name, sizeof(defaultBill.name), "New Bill");
  defaultBill.frequency = MONTHLY;
  defaultBill.payment = 0.0;
  defaultBill.include_in_totals = true;
  defaultBill.locked = false;
  AddEntry(&entryMap, defaultBill);
}

static void DrawBillsTable(const TableLayout *layout, double totals[5],
                           bool *removeRequested, uint64_t *removeKey)
{
  if (!igBeginTable("BillsEntries", 9,
                    ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                        ImGuiTableFlags_Resizable |
                        ImGuiTableFlags_SizingFixedFit |
                        ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
                        ImGuiTableFlags_NoKeepColumnsVisible,
                    (ImVec2){0, layout->tableHeight}, 0))
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

  igEndTable();

  DrawBillTableResizeGrip(layout);
  DrawAddBillButton();
}

static void DrawBillTotals(const double totals[5], const double incomeTotals[5])
{
  static const char *freqLabels[5] = {"Weekly", "Fortnightly", "Monthly",
                                      "Quarterly", "Yearly"};

  igSeparator();
  igSpacing();
  igText("Total");
  igSpacing();

  if (!igBeginTable("TotalsTable", 4,
                    ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                        ImGuiTableFlags_SizingFixedFit,
                    (ImVec2){0, 0}, 0))
    return;

  igTableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 90.0f, 0);
  igTableSetupColumn("Income", ImGuiTableColumnFlags_WidthFixed, 110.0f, 0);
  igTableSetupColumn("Expenses", ImGuiTableColumnFlags_WidthFixed, 110.0f, 0);
  igTableSetupColumn("Net", ImGuiTableColumnFlags_WidthFixed, 110.0f, 0);
  igTableHeadersRow();

  for (int i = 0; i < 5; i++)
  {
    igTableNextRow(0, 0);

    igTableSetColumnIndex(0);
    igText("%s", freqLabels[i]);

    igTableSetColumnIndex(1);
    const char *is = ConvertDoubleToString(incomeTotals[i]);
    igText("$%s", is);
    free((void *)is);

    igTableSetColumnIndex(2);
    const char *es = ConvertDoubleToString(totals[i]);
    igText("$%s", es);
    free((void *)es);

    igTableSetColumnIndex(3);
    double net = incomeTotals[i] - totals[i];
    const char *ns = ConvertDoubleToString(net < 0.0 ? -net : net);
    if (net < 0.0)
      igText("-$%s", ns);
    else
      igText("$%s", ns);
    free((void *)ns);
  }

  igEndTable();
}

void DrawBudgetWindow()
{
  double totals[5] = {0.0};
  bool removeRequested = false;
  uint64_t removeKey = 0;

  double incomeTotals[5] = {0.0};
  bool incomeRemoveRequested = false;
  uint64_t incomeRemoveKey = 0;

  ConfigureBudgetWindowLayout();

  if (!igBegin("Bills", NULL,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoScrollbar |
                   ImGuiWindowFlags_NoScrollWithMouse))
  {
    igEnd();
    return;
  }

  DrawIncomeTable(incomeTotals, &incomeRemoveRequested, &incomeRemoveKey);

  if (incomeRemoveRequested)
    RemoveIncomeEntry(&incomeMap, incomeRemoveKey);

  igSpacing();
  igSeparator();
  igSpacing();

  int entryCount = hmlen(entryMap);
  TableLayout layout = ComputeBillTableLayout(entryCount);
  DrawBillsTable(&layout, totals, &removeRequested, &removeKey);

  if (removeRequested)
  {
    RemoveEntry(&entryMap, removeKey);
  }

  DrawBillTotals(totals, incomeTotals);

  igEnd();
}
