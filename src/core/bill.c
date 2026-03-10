#define STB_DS_IMPLEMENTATION
#include <stb/stb_ds.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "bill.h"
#include "utility.h"

BillEntry *entryMap = NULL;
IncomeEntry *incomeMap = NULL;
char budgetName[MAX_BUDGET_NAME] = {0};
uint64_t _nextID = 0;
uint64_t _nextIncomeID = 0;

const char *ConvertDoubleToString(double value)
{
  bool showCents = value < 10000.0;

  // Split into integer and fractional parts, rounding appropriately
  long long intPart;
  int cents;
  if (showCents)
  {
    long long totalCents = (long long)(value * 100.0 + 0.5);
    intPart = totalCents / 100;
    cents = (int)(totalCents % 100);
  }
  else
  {
    intPart = (long long)(value + 0.5);
    cents = 0;
  }

  // Format the integer part as a plain string, then insert commas
  char intBuf[32];
  snprintf(intBuf, sizeof(intBuf), "%lld", intPart);
  int intLen = strlen(intBuf);

  char withCommas[48];
  int j = 0;
  for (int i = 0; i < intLen; i++)
  {
    if (i > 0 && (intLen - i) % 3 == 0)
      withCommas[j++] = ',';
    withCommas[j++] = intBuf[i];
  }
  withCommas[j] = '\0';

  char result[64];
  if (showCents)
    snprintf(result, sizeof(result), "%s.%02d", withCommas, cents);
  else
    snprintf(result, sizeof(result), "%s", withCommas);

  return strdup(result);
}

bool IsLeapYear(int year)
{
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int DaysInMonth(int month, int year)
{
  static const int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (month == 2 && IsLeapYear(year))
    return 29;
  return days[month];
}

// Returns 0=Mon, 1=Tue, ..., 6=Sun (Tomohiko Sakamoto, adjusted)
int DayOfWeek(int d, int m, int y)
{
  static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  if (m < 3)
    y--;
  int dow = (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7; // 0=Sun
  return (dow + 6) % 7; // convert: 0=Mon, 6=Sun
}

BillDate AddDays(BillDate d, int n)
{
  d.day += n;
  while (d.day > DaysInMonth(d.month, d.year))
  {
    d.day -= DaysInMonth(d.month, d.year);
    d.month++;
    if (d.month > 12)
    {
      d.month = 1;
      d.year++;
    }
  }
  return d;
}

BillDate AddMonths(BillDate d, int n)
{
  d.month += n;
  while (d.month > 12)
  {
    d.month -= 12;
    d.year++;
  }
  int dim = DaysInMonth(d.month, d.year);
  if (d.day > dim)
    d.day = dim;
  return d;
}

BillDate AddYears(BillDate d, int n)
{
  d.year += n;
  int dim = DaysInMonth(d.month, d.year);
  if (d.day > dim)
    d.day = dim;
  return d;
}

static BillDate AdvanceBillDate(const Bill *bill, BillDate d)
{
  switch (bill->frequency)
  {
  case WEEKLY:      return AddDays(d, 7);
  case FORTNIGHTLY: return AddDays(d, 14);
  case MONTHLY:     return AddMonths(d, 1);
  case QUARTERLY:   return AddMonths(d, 3);
  case YEARLY:      return AddYears(d, 1);
  default:          return d;
  }
}

BillDate CalcNextBillDate(const Bill *bill)
{
  BillDate d = {bill->last_date_day, bill->last_date_month, bill->last_date_year};
  if (d.day == 0)
    return d;
  return AdvanceBillDate(bill, d);
}

void BuildYearCalendar(BillEntry *map, int year, int out[12][31],
                       float outColors[12][31][3])
{
  for (int m = 0; m < 12; m++)
    for (int d = 0; d < 31; d++)
    {
      out[m][d] = 0;
      outColors[m][d][0] = 0.0f;
      outColors[m][d][1] = 0.0f;
      outColors[m][d][2] = 0.0f;
    }

  int count = hmlen(map);
  for (int i = 0; i < count; i++)
  {
    Bill *bill = &map[i].value;
    if (!bill->include_in_totals || bill->last_date_day == 0)
      continue;

    BillDate cur = {bill->last_date_day, bill->last_date_month,
                    bill->last_date_year};

    if (cur.year > year)
      continue;

    // Advance until we reach the target year
    while (cur.year < year)
    {
      BillDate next = AdvanceBillDate(bill, cur);
      if (next.year > year)
        break;
      cur = next;
    }
    // One final advance if still before target year
    if (cur.year < year)
    {
      cur = AdvanceBillDate(bill, cur);
      if (cur.year != year)
        continue;
    }

    // Collect all occurrences within target year
    while (cur.year == year)
    {
      int m = cur.month - 1;
      int d = cur.day - 1;
      if (out[m][d] == 0)
      {
        // First bill on this day — store its color
        outColors[m][d][0] = bill->color[0];
        outColors[m][d][1] = bill->color[1];
        outColors[m][d][2] = bill->color[2];
      }
      out[m][d]++;
      cur = AdvanceBillDate(bill, cur);
    }
  }
}

void AddEntry(BillEntry **map, Bill billEntry)
{
  billEntry.include_in_totals = true;
  billEntry.locked = false;
  // Default calendar highlight color: yellow
  if (billEntry.color[0] == 0.0f && billEntry.color[1] == 0.0f && billEntry.color[2] == 0.0f)
  {
    billEntry.color[0] = 1.0f;
    billEntry.color[1] = 0.8f;
    billEntry.color[2] = 0.2f;
  }
  uint64_t newID = _nextID++;
  hmput(*map, newID, billEntry);
  printf("Adding bill entry: %s\nID: %lu\n", billEntry.name, newID);
}

void RemoveEntry(BillEntry **map, uint64_t id)
{
  hmdel(*map, id);
  int entryCount = hmlen(*map);

  Bill *bills = NULL;
  uint64_t *keys = NULL;
  int shiftCount = 0;

  for (int i = 0; i < entryCount; i++)
  {
    if ((*map)[i].key > id)
    {
      bills = realloc(bills, (shiftCount + 1) * sizeof(Bill));
      keys = realloc(keys, (shiftCount + 1) * sizeof(uint64_t));
      bills[shiftCount] = (*map)[i].value;
      keys[shiftCount] = (*map)[i].key;
      shiftCount++;
    }
  }

  for (int i = 0; i < shiftCount; i++)
  {
    hmdel(*map, keys[i]);
    hmput(*map, keys[i] - 1, bills[i]);
  }

  free(bills);
  free(keys);

  _nextID = (hmlen(map) == 0) ? 0 : _nextID - 1;
}

void ClearEntries(BillEntry **map)
{
  hmfree(*map);
  _nextID = 0;
}

void AddIncomeEntry(IncomeEntry **map, Income entry)
{
  entry.enable = true;
  uint64_t newID = _nextIncomeID++;
  hmput(*map, newID, entry);
  printf("Adding income entry: %s\nID: %lu\n", entry.name, newID);
}

void RemoveIncomeEntry(IncomeEntry **map, uint64_t id)
{
  hmdel(*map, id);
  int entryCount = hmlen(*map);

  Income *incomes = NULL;
  uint64_t *keys = NULL;
  int shiftCount = 0;

  for (int i = 0; i < entryCount; i++)
  {
    if ((*map)[i].key > id)
    {
      incomes = realloc(incomes, (shiftCount + 1) * sizeof(Income));
      keys = realloc(keys, (shiftCount + 1) * sizeof(uint64_t));
      incomes[shiftCount] = (*map)[i].value;
      keys[shiftCount] = (*map)[i].key;
      shiftCount++;
    }
  }

  for (int i = 0; i < shiftCount; i++)
  {
    hmdel(*map, keys[i]);
    hmput(*map, keys[i] - 1, incomes[i]);
  }

  free(incomes);
  free(keys);

  _nextIncomeID = (hmlen(map) == 0) ? 0 : _nextIncomeID - 1;
}

void ClearIncomeEntries(IncomeEntry **map)
{
  hmfree(*map);
  _nextIncomeID = 0;
}

double GetIncomeAtFrequency(const Income *income, IncomeFrequency targetFreq)
{
  static const double periodsPerYear[] = {
      52.0, // INCOME_WEEKLY
      26.0, // INCOME_FORTNIGHTLY
      12.0, // INCOME_MONTHLY
      0.0,  // INCOME_ALTERNATING_WEEKLY (handled separately)
  };

  double annualAmount;
  if (income->frequency == INCOME_ALTERNATING_WEEKLY)
  {
    annualAmount = ((income->amount + income->amount_alt) / 2.0) * 52.0;
  }
  else
  {
    annualAmount = income->amount * periodsPerYear[income->frequency];
  }

  return annualAmount / periodsPerYear[targetFreq];
}

const char *GetIncomeFreqName(IncomeFrequency frequency)
{
  switch (frequency)
  {
  case INCOME_WEEKLY:
    return "Weekly";
  case INCOME_FORTNIGHTLY:
    return "Fortnightly";
  case INCOME_MONTHLY:
    return "Monthly";
  case INCOME_ALTERNATING_WEEKLY:
    return "Alternating Weekly";
  default:
    return "Error!";
  }
}

const char *GetBillFreq(PaymentFrequency frequency)
{
  switch (frequency)
  {
  case WEEKLY:
    return "Weekly";
  case FORTNIGHTLY:
    return "Fortnightly";
  case MONTHLY:
    return "Monthly";
  case QUARTERLY:
    return "Quarterly";
  case YEARLY:
    return "Yearly";
  default:
    return "Error!";
  }
}

double ConvertBillPaymentFrequency(const Bill *bill,
                                   PaymentFrequency targetFreq)
{
  // Approximate periods per year for each frequency
  static const double periodsPerYear[] = {
      52.0, // WEEKLY
      26.0, // FORTNIGHTLY
      12.0, // MONTHLY
      4.0,  // QUARTERLY
      1.0   // YEARLY
  };

  double billPeriods = periodsPerYear[bill->frequency];
  double targetPeriods = periodsPerYear[targetFreq];
  double annualAmount = bill->payment * billPeriods;
  return annualAmount / targetPeriods;
}

double TotalBillsByFrequency(BillEntry *map, PaymentFrequency freq)
{
  double total = 0.0;
  int entryCount = hmlen(map);

  for (int i = 0; i < entryCount; i++)
  {
    Bill bill = map[i].value;
    if (!bill.include_in_totals)
    {
      continue;
    }
    total += ConvertBillPaymentFrequency(&bill, freq);
  }

  return total;
}

const char *GetTotalPaymentsByFrequency(BillEntry *map)
{
  static char result[512];
  static const char eBorder[] = "==================================\n";
  static const char *freqNames[] = {
      "Weekly", "Fortnightly", "Monthly", "Quarterly", "Yearly"};
  double totals[5] = {0};
  int entryCount = hmlen(map);
  size_t offset = 0;

  for (int i = 0; i < entryCount; i++)
  {
    Bill bill = map[i].value;
    if (!bill.include_in_totals)
      continue;
    for (int freq = WEEKLY; freq <= YEARLY; freq++)
      totals[freq] += ConvertBillPaymentFrequency(&bill, (PaymentFrequency)freq);
  }

  offset += snprintf(result + offset, sizeof(result) - offset, "%s", eBorder);
  offset += snprintf(result + offset, sizeof(result) - offset,
                     "| %-13s | %-14s |\n", "Frequency", "Expenses");
  offset += snprintf(result + offset, sizeof(result) - offset, "%s", eBorder);

  for (int i = 0; i < 5; i++)
  {
    const char *expStr = ConvertDoubleToString(totals[i]);
    char expBuf[20];
    snprintf(expBuf, sizeof(expBuf), "$%s", expStr);
    offset += snprintf(result + offset, sizeof(result) - offset,
                       "| %-13s | %-14s |\n", freqNames[i], expBuf);
    free((void *)expStr);
  }

  offset += snprintf(result + offset, sizeof(result) - offset, "%s", eBorder);
  return result;
}

const char *GetIncomeMapString(IncomeEntry *map)
{
  static char result[16384];
  static const char wBorder[] =
      "==========================================================================="
      "===========================================================================\n";
  int entryCount = hmlen(map);
  size_t offset = 0;

  offset += snprintf(result + offset, sizeof(result) - offset, "%s", wBorder);
  offset += snprintf(result + offset, sizeof(result) - offset,
                     "| %-20s | %-18s | %-26s | %-12s | %-13s | %-12s | %-12s | %-12s |\n",
                     "Name", "Frequency", "Amount",
                     "Weekly", "Fortnightly", "Monthly", "Quarterly", "Yearly");
  offset += snprintf(result + offset, sizeof(result) - offset, "%s", wBorder);

  for (int i = 0; i < entryCount; i++)
  {
    Income income = map[i].value;

    double w = GetIncomeAtFrequency(&income, INCOME_WEEKLY);
    double f = GetIncomeAtFrequency(&income, INCOME_FORTNIGHTLY);
    double m = GetIncomeAtFrequency(&income, INCOME_MONTHLY);
    double q = w * 13.0;
    double y = w * 52.0;

    const char *ws = ConvertDoubleToString(w);
    const char *fs = ConvertDoubleToString(f);
    const char *ms = ConvertDoubleToString(m);
    const char *qs = ConvertDoubleToString(q);
    const char *ys = ConvertDoubleToString(y);

    char amountBuf[32];
    if (income.frequency == INCOME_ALTERNATING_WEEKLY)
    {
      const char *a1 = ConvertDoubleToString(income.amount);
      const char *a2 = ConvertDoubleToString(income.amount_alt);
      snprintf(amountBuf, sizeof(amountBuf), "W1:$%-10s W2:$%-7s", a1, a2);
      free((void *)a1);
      free((void *)a2);
    }
    else
    {
      const char *as = ConvertDoubleToString(income.amount);
      snprintf(amountBuf, sizeof(amountBuf), "$%-25s", as);
      free((void *)as);
    }

    char rowName[32];
    if (!income.enable)
      snprintf(rowName, sizeof(rowName), "%-18s *", income.name);
    else
      snprintf(rowName, sizeof(rowName), "%s", income.name);

    char wBuf[16], fBuf[16], mBuf[16], qBuf[16], yBuf[16];
    snprintf(wBuf, sizeof(wBuf), "$%s", ws);
    snprintf(fBuf, sizeof(fBuf), "$%s", fs);
    snprintf(mBuf, sizeof(mBuf), "$%s", ms);
    snprintf(qBuf, sizeof(qBuf), "$%s", qs);
    snprintf(yBuf, sizeof(yBuf), "$%s", ys);

    offset += snprintf(result + offset, sizeof(result) - offset,
                       "| %-20s | %-18s | %-26s | %-12s | %-13s | %-12s | %-12s | %-12s |\n",
                       rowName, GetIncomeFreqName(income.frequency), amountBuf,
                       wBuf, fBuf, mBuf, qBuf, yBuf);

    free((void *)ws);
    free((void *)fs);
    free((void *)ms);
    free((void *)qs);
    free((void *)ys);
  }

  offset += snprintf(result + offset, sizeof(result) - offset, "%s", wBorder);
  return result;
}

const char *GetEntryMapString(BillEntry *map)
{
  static char result[65536];
  static const char wBorder[] =
      "==========================================================================="
      "===========================================================================\n";
  static const char nBorder[] =
      "====================================================================\n";
  static const char *freqNames[] = {
      "Weekly", "Fortnightly", "Monthly", "Quarterly", "Yearly"};

  int billCount = hmlen(map);
  int incomeCount = hmlen(incomeMap);
  const char *name = TrimExt(budgetName);
  size_t offset = 0;

  double incomeTotals[5] = {0};
  double billTotals[5] = {0};

  // Title block
  {
    int nameLen = (int)strlen(name);
    int innerWidth = 146;
    int leftPad = (innerWidth - nameLen) / 2;
    int rightPad = innerWidth - nameLen - leftPad;
    offset += snprintf(result + offset, sizeof(result) - offset, "%s", wBorder);
    offset += snprintf(result + offset, sizeof(result) - offset,
                       "| %*s%s%*s |\n", leftPad, "", name, rightPad, "");
    offset += snprintf(result + offset, sizeof(result) - offset, "%s\n", wBorder);
  }

  // Income section (only if there are any incomes)
  if (incomeCount > 0)
  {
    offset += snprintf(result + offset, sizeof(result) - offset, "Income\n");
    offset += snprintf(result + offset, sizeof(result) - offset, "%s", wBorder);
    offset += snprintf(result + offset, sizeof(result) - offset,
                       "| %-20s | %-18s | %-26s | %-12s | %-13s | %-12s | %-12s | %-12s |\n",
                       "Name", "Frequency", "Amount",
                       "Weekly", "Fortnightly", "Monthly", "Quarterly", "Yearly");
    offset += snprintf(result + offset, sizeof(result) - offset, "%s", wBorder);

    for (int i = 0; i < incomeCount; i++)
    {
      Income income = incomeMap[i].value;

      double w = GetIncomeAtFrequency(&income, INCOME_WEEKLY);
      double f = GetIncomeAtFrequency(&income, INCOME_FORTNIGHTLY);
      double m = GetIncomeAtFrequency(&income, INCOME_MONTHLY);
      double q = w * 13.0;
      double y = w * 52.0;

      if (income.enable)
      {
        incomeTotals[0] += w;
        incomeTotals[1] += f;
        incomeTotals[2] += m;
        incomeTotals[3] += q;
        incomeTotals[4] += y;
      }

      const char *ws = ConvertDoubleToString(w);
      const char *fs = ConvertDoubleToString(f);
      const char *ms = ConvertDoubleToString(m);
      const char *qs = ConvertDoubleToString(q);
      const char *ys = ConvertDoubleToString(y);

      char amountBuf[32];
      if (income.frequency == INCOME_ALTERNATING_WEEKLY)
      {
        const char *a1 = ConvertDoubleToString(income.amount);
        const char *a2 = ConvertDoubleToString(income.amount_alt);
        snprintf(amountBuf, sizeof(amountBuf), "W1:$%-10s W2:$%-7s", a1, a2);
        free((void *)a1);
        free((void *)a2);
      }
      else
      {
        const char *as = ConvertDoubleToString(income.amount);
        snprintf(amountBuf, sizeof(amountBuf), "$%-25s", as);
        free((void *)as);
      }

      char rowName[32];
      if (!income.enable)
        snprintf(rowName, sizeof(rowName), "%-18s *", income.name);
      else
        snprintf(rowName, sizeof(rowName), "%s", income.name);

      char wBuf[16], fBuf[16], mBuf[16], qBuf[16], yBuf[16];
      snprintf(wBuf, sizeof(wBuf), "$%s", ws);
      snprintf(fBuf, sizeof(fBuf), "$%s", fs);
      snprintf(mBuf, sizeof(mBuf), "$%s", ms);
      snprintf(qBuf, sizeof(qBuf), "$%s", qs);
      snprintf(yBuf, sizeof(yBuf), "$%s", ys);

      offset += snprintf(result + offset, sizeof(result) - offset,
                         "| %-20s | %-18s | %-26s | %-12s | %-13s | %-12s | %-12s | %-12s |\n",
                         rowName, GetIncomeFreqName(income.frequency), amountBuf,
                         wBuf, fBuf, mBuf, qBuf, yBuf);

      free((void *)ws);
      free((void *)fs);
      free((void *)ms);
      free((void *)qs);
      free((void *)ys);
    }

    offset += snprintf(result + offset, sizeof(result) - offset, "%s\n", wBorder);
  }

  // Expenses section
  offset += snprintf(result + offset, sizeof(result) - offset, "Expenses\n");
  offset += snprintf(result + offset, sizeof(result) - offset, "%s", wBorder);
  offset += snprintf(result + offset, sizeof(result) - offset,
                     "| %-20s | %-18s | %-26s | %-12s | %-13s | %-12s | %-12s | %-12s |\n",
                     "Name", "Frequency", "Amount",
                     "Weekly", "Fortnightly", "Monthly", "Quarterly", "Yearly");
  offset += snprintf(result + offset, sizeof(result) - offset, "%s", wBorder);

  for (int i = 0; i < billCount; i++)
  {
    Bill bill = map[i].value;

    double w = ConvertBillPaymentFrequency(&bill, WEEKLY);
    double f = ConvertBillPaymentFrequency(&bill, FORTNIGHTLY);
    double m = ConvertBillPaymentFrequency(&bill, MONTHLY);
    double q = ConvertBillPaymentFrequency(&bill, QUARTERLY);
    double y = ConvertBillPaymentFrequency(&bill, YEARLY);

    if (bill.include_in_totals)
    {
      billTotals[0] += w;
      billTotals[1] += f;
      billTotals[2] += m;
      billTotals[3] += q;
      billTotals[4] += y;
    }

    const char *as = ConvertDoubleToString(bill.payment);
    const char *ws = ConvertDoubleToString(w);
    const char *fs = ConvertDoubleToString(f);
    const char *ms = ConvertDoubleToString(m);
    const char *qs = ConvertDoubleToString(q);
    const char *ys = ConvertDoubleToString(y);

    char amountBuf[32];
    snprintf(amountBuf, sizeof(amountBuf), "$%-25s", as);

    char rowName[32];
    if (!bill.include_in_totals)
      snprintf(rowName, sizeof(rowName), "%-18s *", bill.name);
    else
      snprintf(rowName, sizeof(rowName), "%s", bill.name);

    char wBuf[16], fBuf[16], mBuf[16], qBuf[16], yBuf[16];
    snprintf(wBuf, sizeof(wBuf), "$%s", ws);
    snprintf(fBuf, sizeof(fBuf), "$%s", fs);
    snprintf(mBuf, sizeof(mBuf), "$%s", ms);
    snprintf(qBuf, sizeof(qBuf), "$%s", qs);
    snprintf(yBuf, sizeof(yBuf), "$%s", ys);

    offset += snprintf(result + offset, sizeof(result) - offset,
                       "| %-20s | %-18s | %-26s | %-12s | %-13s | %-12s | %-12s | %-12s |\n",
                       rowName, GetBillFreq(bill.frequency), amountBuf,
                       wBuf, fBuf, mBuf, qBuf, yBuf);

    free((void *)as);
    free((void *)ws);
    free((void *)fs);
    free((void *)ms);
    free((void *)qs);
    free((void *)ys);
  }

  offset += snprintf(result + offset, sizeof(result) - offset, "%s\n", wBorder);

  // Totals section
  offset += snprintf(result + offset, sizeof(result) - offset, "Total\n");
  offset += snprintf(result + offset, sizeof(result) - offset, "%s", nBorder);
  offset += snprintf(result + offset, sizeof(result) - offset,
                     "| %-13s | %-14s | %-14s | %-14s |\n",
                     "Frequency", "Income", "Expenses", "Net");
  offset += snprintf(result + offset, sizeof(result) - offset, "%s", nBorder);

  for (int i = 0; i < 5; i++)
  {
    double net = incomeTotals[i] - billTotals[i];
    const char *incStr = ConvertDoubleToString(incomeTotals[i]);
    const char *expStr = ConvertDoubleToString(billTotals[i]);
    double absNet = net < 0.0 ? -net : net;
    const char *netStr = ConvertDoubleToString(absNet);

    char incBuf[20], expBuf[20], netBuf[24];
    snprintf(incBuf, sizeof(incBuf), "$%s", incStr);
    snprintf(expBuf, sizeof(expBuf), "$%s", expStr);
    if (net < 0.0)
      snprintf(netBuf, sizeof(netBuf), "-$%s", netStr);
    else
      snprintf(netBuf, sizeof(netBuf), "$%s", netStr);

    offset += snprintf(result + offset, sizeof(result) - offset,
                       "| %-13s | %-14s | %-14s | %-14s |\n",
                       freqNames[i], incBuf, expBuf, netBuf);

    free((void *)incStr);
    free((void *)expStr);
    free((void *)netStr);
  }

  offset += snprintf(result + offset, sizeof(result) - offset, "%s", nBorder);

  return result;
}

void PrintEntryMap(BillEntry *map) { printf("%s", GetEntryMapString(map)); }
