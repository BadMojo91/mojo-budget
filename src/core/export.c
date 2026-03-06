#include "export.h"
#include "bill.h"
#include "utility.h"
#include <errno.h>
#include <stb/stb_ds.h>
#include <stdio.h>

char savePath[4096] = {0};

void SetSavePath(const char *path)
{
  strncpy(savePath, path, sizeof(savePath) - 1);
  savePath[sizeof(savePath) - 1] = '\0';
}

BillEntry *BudgetLoad(const char *filePath)
{
  FILE *file = fopen(filePath, "rb");

  if (CheckFile(file, filePath, "load") != 0)
    return NULL;

  SetSavePath(filePath);

  const char *trimmedPath = TrimHomePath(filePath);

  if (fread(budgetName, sizeof(budgetName), 1, file) != 1)
  {
    fprintf(stderr, "Error: failed to read budget name from '%s'\n", filePath);
    fclose(file);
    return NULL;
  }

  printf("Loading budget: %s\n", trimmedPath);
  int entryCount;

  if (fread(&entryCount, sizeof(int), 1, file) != 1 || entryCount < 0)
  {
    fprintf(stderr, "Error: failed to read entry count from '%s'\n", filePath);
    fclose(file);
    return NULL;
  }

  BillEntry *map = NULL;
  for (int i = 0; i < entryCount; i++)
  {
    uint64_t key;
    Bill bill;
    if (fread(&key, sizeof(uint64_t), 1, file) != 1 ||
        fread(&bill, sizeof(Bill), 1, file) != 1)
    {
      fprintf(stderr, "Error: unexpected EOF reading bill %d from '%s'\n", i, filePath);
      hmfree(map);
      fclose(file);
      return NULL;
    }
    hmput(map, key, bill);
    _nextID++;
    // printf("Loaded bill entry: %s\nID: %lu\n", bill.name, key);
  }
  _nextID++;
  printf("Loaded %d bills.\n", entryCount);
  fclose(file);
  return map;
}

void BudgetSave(BillEntry *entryMap, const char *filePath)
{
  const char *fileName = TrimPath(filePath);
  const char *trimmedPath = TrimHomePath(filePath);

  snprintf(budgetName, sizeof(budgetName), "%s", fileName);
  //strcpy(budgetName, fileName);
  FILE *file = fopen(filePath, "wb");

  if (CheckFile(file, filePath, "save") != 0)
    return;

  SetSavePath(filePath);

  fwrite(budgetName, sizeof(budgetName), 1, file);
  printf("Writing budget: %s\n", trimmedPath);
  int entryCount = hmlen(entryMap);
  fwrite(&entryCount, sizeof(int), 1, file);
  for (int i = 0; i < entryCount; i++)
  {
    fwrite(&entryMap[i].key, sizeof(uint64_t), 1, file);
    fwrite(&entryMap[i].value, sizeof(Bill), 1, file);
  }
  fclose(file);
}

void ExportAsTXT(BillEntry *entryMap, const char *filePath)
{
  const char *data = GetEntryMapString(entryMap);
  FILE *file = fopen(filePath, "w");

  if (CheckFile(file, filePath, "export") != 0)
    return;

  fprintf(file, "%s", data);
  fclose(file);
}

static void WriteCSVField(FILE *file, const char *s)
{
  fputc('"', file);
  for (; *s; s++) {
    if (*s == '"') fputc('"', file);
    fputc(*s, file);
  }
  fputc('"', file);
}

void ExportAsCSV(BillEntry *entryMap, const char *filePath)
{
  FILE *file = fopen(filePath, "w");

  if (CheckFile(file, filePath, "export") != 0)
    return;

  int billCount = hmlen(entryMap);
  fprintf(
      file,
      "ID, Name, Frequency, Amount, Week, Fortnight, Month, Quarter, Year\n");

  for (int i = 0; i < billCount; i++)
  {
    Bill bill = (entryMap)[i].value;
    uint64_t id = (entryMap)[i].key;
    double w, f, m, q, y;
    w = ConvertBillPaymentFrequency(&bill, WEEKLY);
    f = ConvertBillPaymentFrequency(&bill, FORTNIGHTLY);
    m = ConvertBillPaymentFrequency(&bill, MONTHLY);
    q = ConvertBillPaymentFrequency(&bill, QUARTERLY);
    y = ConvertBillPaymentFrequency(&bill, YEARLY);

    fprintf(file, "%lu, ", id);
    WriteCSVField(file, bill.name);
    fprintf(file, ", %s, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n",
            GetBillFreq(bill.frequency), bill.payment, w, f, m, q, y);
  }

  fclose(file);
}
