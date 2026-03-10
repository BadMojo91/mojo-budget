#include "ui_core.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../core/bill.h"
#include "cimgui.h"
#include <stb/stb_ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char *g_freqNames[] = {"Weekly", "Fortnightly", "Monthly",
                                    "Quarterly", "Yearly"};
static const char *g_incomeFreqNames[] = {"Weekly", "Fortnightly", "Monthly",
                                          "Alternating Weekly"};
static float g_billTableHeight = 0.0f;
static float g_incomeTableHeight = 0.0f;
static int g_calYear = 0;
bool g_showCalendar = false;

#define CALENDAR_WIDTH 680.0f

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
  if (g_showCalendar)
    panelSize.x -= CALENDAR_WIDTH;

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
  igTableSetupColumn("Next Due", ImGuiTableColumnFlags_WidthFixed, 90.0f, 0);
  igTableSetupColumn("Parameters",
                     ImGuiTableColumnFlags_WidthFixed |
                         ImGuiTableColumnFlags_NoResize |
                         ImGuiTableColumnFlags_NoHide,
                     210, 0);
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

static void DrawBillNextDueColumn(Bill *bill)
{
  static const char *monthNames[] = {"Jan","Feb","Mar","Apr","May","Jun",
                                      "Jul","Aug","Sep","Oct","Nov","Dec"};
  igTableSetColumnIndex(8);
  BillDate d = CalcNextBillDate(bill);
  if (d.day == 0)
  {
    igText("---");
  }
  else
  {
    igText("%02d %s %d", d.day, monthNames[d.month - 1], d.year);
  }
}

static void DrawBillActions(BillEntry *entry, bool *removeRequested,
                            uint64_t *removeKey)
{
  Bill *bill = &entry->value;
  igTableSetColumnIndex(9);

  static int dp_day = 0, dp_month = 0, dp_year = 2024;
  static const char *dpMonthNames[] = {"Jan","Feb","Mar","Apr","May","Jun",
                                        "Jul","Aug","Sep","Oct","Nov","Dec"};
  static const char *dayItems[31] = {
    "01","02","03","04","05","06","07","08","09","10",
    "11","12","13","14","15","16","17","18","19","20",
    "21","22","23","24","25","26","27","28","29","30","31"
  };

  const char *useLabel = bill->include_in_totals ? "On" : "Off";
  const char *lockLabel = bill->locked ? "Unlock" : "Lock";
  const float spacing = 4.0f;

  if (igSmallButton(useLabel))
    bill->include_in_totals = !bill->include_in_totals;

  igSameLine(0.0f, spacing);

  if (igSmallButton(lockLabel))
    bill->locked = !bill->locked;

  igSameLine(0.0f, spacing);

  if (igSmallButton("Date"))
  {
    if (bill->last_date_day > 0)
    {
      dp_day   = bill->last_date_day - 1;
      dp_month = bill->last_date_month - 1;
      dp_year  = bill->last_date_year;
    }
    else
    {
      time_t t = time(NULL);
      struct tm *tm_info = localtime(&t);
      dp_day   = tm_info->tm_mday - 1;
      dp_month = tm_info->tm_mon;
      dp_year  = tm_info->tm_year + 1900;
    }
    igOpenPopup_Str("##datepick", 0);
  }

  if (igBeginPopup("##datepick", 0))
  {
    int daysInM = DaysInMonth(dp_month + 1, dp_year);
    if (dp_day >= daysInM) dp_day = daysInM - 1;

    igSetNextItemWidth(60.0f);
    igCombo_Str_arr("##dpmonth", &dp_month, dpMonthNames, 12, 12);
    igSameLine(0.0f, 4.0f);
    igSetNextItemWidth(50.0f);
    igCombo_Str_arr("##dpday", &dp_day, dayItems, daysInM, daysInM);
    igSameLine(0.0f, 4.0f);
    igSetNextItemWidth(90.0f);
    igInputInt("##dpyear", &dp_year, 1, 10, 0);

    if (igSmallButton("Set"))
    {
      bill->last_date_day   = dp_day + 1;
      bill->last_date_month = dp_month + 1;
      bill->last_date_year  = dp_year;
      igCloseCurrentPopup();
    }
    igSameLine(0.0f, 4.0f);
    if (igSmallButton("Clear"))
    {
      bill->last_date_day = 0;
      igCloseCurrentPopup();
    }
    igEndPopup();
  }

  igSameLine(0.0f, spacing);

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
  DrawBillNextDueColumn(bill);
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
  if (!igBeginTable("BillsEntries", 10,
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

static void ShowBillsOnDate(BillEntry *map, int year, int month, int day)
{
  int n = hmlen(map);
  int found = 0;
  double total = 0.0;
  for (int i = 0; i < n; i++)
  {
    Bill *bill = &map[i].value;
    if (!bill->include_in_totals || bill->last_date_day == 0)
      continue;
    BillDate cur = {bill->last_date_day, bill->last_date_month, bill->last_date_year};
    for (int iter = 0; iter < 400; iter++)
    {
      if (cur.year > year) break;
      if (cur.year == year && cur.month == month && cur.day == day)
      {
        igText("%-28s  $%.2f", bill->name, bill->payment);
        found++;
        total += bill->payment;
        break;
      }
      switch (bill->frequency)
      {
        case WEEKLY:       cur = AddDays(cur, 7);    break;
        case FORTNIGHTLY:  cur = AddDays(cur, 14);   break;
        case MONTHLY:      cur = AddMonths(cur, 1);  break;
        case QUARTERLY:    cur = AddMonths(cur, 3);  break;
        case YEARLY:       cur = AddYears(cur, 1);   break;
        default: iter = 400; break;
      }
    }
  }
  if (found == 0)
    igText("Bills due: (none)");
  else if (found > 1)
  {
    igSeparator();
    igText("Total: $%.2f", total);
  }
}

static void DrawYearCalendar(BillEntry *map)
{
  if (g_calYear == 0)
  {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    g_calYear = tm_info->tm_year + 1900;
  }

  static const char *monthNames[] = {"Jan","Feb","Mar","Apr","May","Jun",
                                      "Jul","Aug","Sep","Oct","Nov","Dec"};

  if (igSmallButton("<")) g_calYear--;
  igSameLine(0.0f, 8.0f);
  igText("%d", g_calYear);
  igSameLine(0.0f, 8.0f);
  if (igSmallButton(">")) g_calYear++;

  int calData[12][31];
  memset(calData, 0, sizeof(calData));
  BuildYearCalendar(map, g_calYear, calData);

  if (!igBeginTable("CalGrid", 3,
                    ImGuiTableFlags_SizingStretchSame,
                    (ImVec2){0, 0}, 0))
    return;

  igPushStyleVar_Vec2(ImGuiStyleVar_CellPadding, (ImVec2){3.0f, 3.0f});

  for (int m = 0; m < 12; m++)
  {
    if (m % 3 == 0)
      igTableNextRow(0, 0);
    igTableSetColumnIndex(m % 3);

    igPushID_Int(m + 200);
    igPushStyleVar_Vec2(ImGuiStyleVar_ItemSpacing, (ImVec2){2.0f, 1.0f});
    igBeginChild_Str("##mchild", (ImVec2){0, 145}, false,
                     ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    float cellW = igGetContentRegionAvail().x;
    float textW = igCalcTextSize(monthNames[m], NULL, false, -1.0f).x;
    float cx = igGetCursorPosX() + (cellW - textW) * 0.5f;
    if (cx > igGetCursorPosX()) igSetCursorPosX(cx);

    // Subtle highlight behind month name
    ImVec2 textPos = igGetCursorScreenPos();
    float padX = 6.0f, padY = 2.0f;
    ImVec2 rectMin = {textPos.x - padX, textPos.y - padY};
    ImVec2 rectMax = {textPos.x + textW + padX, textPos.y + igGetTextLineHeight() + padY};
    ImDrawList *dl = igGetWindowDrawList();
    ImDrawList_AddRectFilled(dl, rectMin, rectMax,
                             igColorConvertFloat4ToU32((ImVec4){0.3f, 0.5f, 0.8f, 0.25f}),
                             3.0f, 0);
    igPushStyleColor_Vec4(ImGuiCol_Text, (ImVec4){0.7f, 0.88f, 1.0f, 1.0f});
    igText("%s", monthNames[m]);
    igPopStyleColor(1);
    igSeparator();

    igPushStyleColor_Vec4(ImGuiCol_TableBorderLight, (ImVec4){1.0f, 1.0f, 1.0f, 0.07f});
    if (igBeginTable("M", 7,
                     ImGuiTableFlags_SizingFixedSame |
                     ImGuiTableFlags_BordersInner,
                     (ImVec2){0, 0}, 0))
    {
      for (int c = 0; c < 7; c++)
        igTableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 20.0f, 0);

      static const char *dowNames[] = {"Mo","Tu","We","Th","Fr","Sa","Su"};
      igTableNextRow(0, 0);
      for (int c = 0; c < 7; c++)
      {
        igTableSetColumnIndex(c);
        igText("%s", dowNames[c]);
      }

      int startDow  = DayOfWeek(1, m + 1, g_calYear);
      int daysInMonth = DaysInMonth(m + 1, g_calYear);
      int col = startDow;
      igTableNextRow(0, 0);

      for (int day = 1; day <= daysInMonth; day++)
      {
        if (col >= 7)
        {
          igTableNextRow(0, 0);
          col = 0;
        }
        igTableSetColumnIndex(col);

        bool hasBill = calData[m][day - 1] > 0;
        if (hasBill)
        {
          ImVec4 yellow = {1.0f, 0.8f, 0.2f, 1.0f};
          igPushStyleColor_Vec4(ImGuiCol_Text, yellow);
        }
        igText("%d", day);
        if (hasBill)
          igPopStyleColor(1);

        if (hasBill && igIsItemHovered(ImGuiHoveredFlags_None))
        {
          if (igBeginTooltip())
          {
            ShowBillsOnDate(map, g_calYear, m + 1, day);
            igEndTooltip();
          }
        }

        col++;
      }

      igEndTable();
    }
    igPopStyleColor(1); // TableBorderLight

    igEndChild();
    igPopStyleVar(1); // ItemSpacing
    igPopID();
  }

  igPopStyleVar(1); // CellPadding
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

void DrawCalendarWindow()
{
  if (!g_showCalendar)
    return;

  ImGuiViewport *viewport = igGetMainViewport();
  float menuBarHeight = igGetFrameHeight();

  igSetNextWindowPos(
      (ImVec2){viewport->Pos.x + viewport->Size.x - CALENDAR_WIDTH,
               viewport->Pos.y + menuBarHeight},
      ImGuiCond_Always, (ImVec2){0, 0});
  igSetNextWindowSize(
      (ImVec2){CALENDAR_WIDTH, viewport->Size.y - menuBarHeight},
      ImGuiCond_Always);

  if (igBegin("Calendar", &g_showCalendar,
               ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoCollapse))
  {
    DrawYearCalendar(entryMap);
  }
  igEnd();
}
