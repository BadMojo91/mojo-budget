#define STB_DS_IMPLEMENTATION
#include <stb/stb_ds.h>
#include <stddef.h>
#include <stdio.h>

#include "bill.h"

BillEntry* entryMap = NULL;

void AddEntry(BillEntry** map, Bill billEntry)
{
  billEntry.include_in_totals = true;
  billEntry.locked = false;
  uint64_t newID = _nextID++;
  hmput(*map, newID, billEntry);
  printf("Adding bill entry: %s\nID: %lu\n", billEntry.name, newID);
}

void RemoveEntry(BillEntry** map, uint64_t id)
{
  int entryCount = hmlen(*map);
  hmdel(*map, id);

  // Shift keys after the removed id
  for (int i = 0; i < entryCount; i++)
  {
    if ((*map)[i].key > id)
    {
      Bill bill = (*map)[i].value;
      uint64_t oldKey = (*map)[i].key;
      hmdel(*map, oldKey);
      hmput(*map, oldKey - 1, bill);
    }
  }

  // Update _nextID to be highest key + 1
  uint64_t maxKey = 0;
  entryCount = hmlen(*map);
  for (int i = 0; i < entryCount; i++)
  {
    if ((*map)[i].key > maxKey)
    {
      maxKey = (*map)[i].key;
    }
  }
  _nextID = maxKey + 1;
}

void ClearEntries(BillEntry** map)
{
  hmfree(*map);
  _nextID = 0;
}

const char* GetBillFreq(PaymentFrequency frequency)
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

double ConvertBillPaymentFrequency(const Bill* bill,
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

const char* GetTotalPaymentsByFrequency(BillEntry* map)
{
  static char result[512];
  double totals[5] = { 0 }; // WEEKLY, FORTNIGHTLY, MONTHLY, QUARTERLY, YEARLY
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

  snprintf(result, sizeof(result),
    "Weekly: $%.2f\nFortnightly: $%.2f\nMonthly: $%.2f\nQuarterly: "
    "$%.2f\nYearly: $%.2f\n",
    totals[WEEKLY], totals[FORTNIGHTLY], totals[MONTHLY],
    totals[QUARTERLY], totals[YEARLY]);

  return result;
}

const char* GetEntryMapString(BillEntry* map)
{
  static char result[4096];
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
void PrintEntryMap(BillEntry* map) { printf("%s", GetEntryMapString(map)); }

BillEntry* LoadEntryMap(const char* file)
{
  FILE* fp = fopen(file, "rb");
  if (!fp)
    return NULL;

  int entryCount;
  fread(&entryCount, sizeof(int), 1, fp);

  BillEntry* map = NULL;
  for (int i = 0; i < entryCount; i++)
  {
    uint64_t key;
    Bill bill;
    fread(&key, sizeof(uint64_t), 1, fp);
    fread(&bill, sizeof(Bill), 1, fp);
    hmput(map, key, bill);
  }
  fclose(fp);
  return map;
}

void SaveEntryMap(const char* file, BillEntry* map, SaveFileType type)
{
  switch (type)
  {
  case FILETYPE_TXT:
    const char* data = GetEntryMapString(map);
    FILE* fp = fopen(file, "w");
    if (fp)
    {
      fprintf(fp, "%s", data);
      fclose(fp);
    }
    break;

  case FILETYPE_BUD:
    FILE* fd = fopen(file, "wb");
    if (fd)
    {
      int entryCount = hmlen(map);
      fwrite(&entryCount, sizeof(int), 1, fd);
      for (int i = 0; i < entryCount; i++)
      {
        fwrite(&entryMap[i].key, sizeof(uint64_t), 1, fd);
        fwrite(&entryMap[i].value, sizeof(Bill), 1, fd);
      }
      fclose(fd);
    }
    break;
  }
}
