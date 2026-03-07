#define STB_DS_IMPLEMENTATION
#include <stb/stb_ds.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "bill.h"
#include "utility.h"


BillEntry *entryMap = NULL;
char budgetName[MAX_BUDGET_NAME] = {0};
uint64_t _nextID = 0;

const char* ConvertDoubleToString(double value)
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

void AddEntry(BillEntry **map, Bill billEntry)
{
  billEntry.include_in_totals = true;
  billEntry.locked = false;
  uint64_t newID = _nextID++;
  hmput(*map, newID, billEntry);
  printf("Adding bill entry: %s\nID: %lu\n", billEntry.name, newID);
}

void RemoveEntry(BillEntry **map, uint64_t id)
{
  hmdel(*map, id);
  int entryCount = hmlen(*map);
  
  Bill* bills = NULL;
  uint64_t *keys = NULL;
  int shiftCount = 0;

  for(int i = 0; i < entryCount; i++)
  {
    if((*map)[i].key > id)
    {
      bills = realloc(bills, (shiftCount + 1) * sizeof(Bill));
      keys = realloc(keys, (shiftCount + 1) * sizeof(uint64_t));
      bills[shiftCount] = (*map)[i].value;
      keys[shiftCount] = (*map)[i].key;
      shiftCount++;
    }
  }

  for(int i = 0; i < shiftCount; i++)
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
  double totals[5] = {0}; // WEEKLY, FORTNIGHTLY, MONTHLY, QUARTERLY, YEARLY
  int entryCount = hmlen(map);

  for (int i = 0; i < entryCount; i++)
  {
    Bill bill = map[i].value;
    if (!bill.include_in_totals)
    {
      continue;
    }
    for (int freq = WEEKLY; freq <= YEARLY; freq++)
    {
      totals[freq] += ConvertBillPaymentFrequency(&bill, freq);
    }
  }
  int numOfFreqs = 5;
  const char* totalString[numOfFreqs];
  for(int i = 0; i < 5; i++)
  {
    totalString[i] = GetBillFreq(totals[i]);
  }


  snprintf(result, sizeof(result),
         "Weekly: $%s\nFortnightly: $%s\nMonthly: $%s\nQuarterly: $%s\nYearly: $%s\n",
         totalString[WEEKLY], totalString[FORTNIGHTLY], totalString[MONTHLY],
         totalString[QUARTERLY], totalString[YEARLY]);

  return result;
}

const char *GetEntryMapString(BillEntry *map)
{
  static char result[32768]; // 32KB buffer should hold over 200 entries comfortably
  int entryCount = hmlen(map);
  char line[256];

  size_t offset = 0;
  //  offset += snprintf(result + offset, sizeof(result) - offset, "Found
  //  entries: %d\n\n", entryCount);
  offset += snprintf(result + offset, sizeof(result) - offset,
                     "---------------------------------------------------\n");
  offset += snprintf(result + offset, sizeof(result) - offset,
                     "-                      BILLS                      -\n");
  offset += snprintf(result + offset, sizeof(result) - offset,
                     "---------------------------------------------------\n\n");
  offset +=
      snprintf(result + offset, sizeof(result) - offset,
               "ID | Name               | Frequency      | Amount    | Week    "
               "  | Fortnight   | Month     | Quarter   | Year      |\n");
  offset +=
      snprintf(result + offset, sizeof(result) - offset,
               "---------------------------------------------------------------"
               "------------------------------------------------\n");

  for (int i = 0; i < entryCount; i++)
  {
    Bill bill = map[i].value;
    uint64_t id = map[i].key;
    double w, f, m, q, y;
    w = ConvertBillPaymentFrequency(&bill, WEEKLY);
    f = ConvertBillPaymentFrequency(&bill, FORTNIGHTLY);
    m = ConvertBillPaymentFrequency(&bill, MONTHLY);
    q = ConvertBillPaymentFrequency(&bill, QUARTERLY);
    y = ConvertBillPaymentFrequency(&bill, YEARLY);

    snprintf(line, sizeof(line),
             "%-2lu | %-18s | %-14s | $%8.2f | $%8.2f | $%10.2f | $%8.2f | "
             "$%8.2f | $%8.2f |\n",
             id, bill.name, GetBillFreq(bill.frequency), bill.payment, w, f, m,
             q, y);

    offset += snprintf(result + offset, sizeof(result) - offset, "%s", line);
  }

  offset +=
      snprintf(result + offset, sizeof(result) - offset,
               "\n\n---------------------------------------------------\n");
  offset += snprintf(result + offset, sizeof(result) - offset, "%s",
                     GetTotalPaymentsByFrequency(map));
  offset += snprintf(result + offset, sizeof(result) - offset,
                     "---------------------------------------------------\n\n");

  return result;
}

void PrintEntryMap(BillEntry *map) { printf("%s", GetEntryMapString(map)); }

