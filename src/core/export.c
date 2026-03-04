#include "export.h"
#include "bill.h"
#include "utility.h"
#include <stdio.h>

#include <stb/stb_ds.h>

char savePath[4096] = {0};

void SetSavePath(const char *path)
{
  strncpy(savePath, path, sizeof(savePath) - 1);
  savePath[sizeof(savePath) - 1] = '\0';
}

BillEntry *BudgetLoad(const char *filePath)
{
  FILE *fp = fopen(filePath, "rb");
  if (!fp)
    return NULL;

  SetSavePath(filePath);

  const char* trimmedPath = TrimHomePath(filePath);
  fread(budgetName, sizeof(budgetName), 1, fp);
  printf("Loading budget: %s\n", trimmedPath);
  int entryCount;
  fread(&entryCount, sizeof(int), 1, fp);

  BillEntry *map = NULL;
  for (int i = 0; i < entryCount; i++)
  {
    uint64_t key;
    Bill bill;
    fread(&key, sizeof(uint64_t), 1, fp);
    fread(&bill, sizeof(Bill), 1, fp);
    hmput(map, key, bill);
    _nextID++;
    //printf("Loaded bill entry: %s\nID: %lu\n", bill.name, key);
  }
  printf("Loaded %d bills.\n", entryCount);
  fclose(fp);
  return map;
}

void BudgetSave(BillEntry* entryMap, const char *filePath)
{
  const char *fileName = TrimPath(filePath);
  const char *trimmedPath = TrimHomePath(filePath);

  strcpy(budgetName, fileName);
    FILE *fd = fopen(filePath, "wb");
    if (fd)
    {
      SetSavePath(filePath);

      fwrite(budgetName, sizeof(budgetName), 1, fd);
      printf("Writing budget: %s\n", trimmedPath);
      int entryCount = hmlen(entryMap);
      fwrite(&entryCount, sizeof(int), 1, fd);
      for (int i = 0; i < entryCount; i++)
      {
        fwrite(&entryMap[i].key, sizeof(uint64_t), 1, fd);
        fwrite(&entryMap[i].value, sizeof(Bill), 1, fd);
      }
      fclose(fd);
    }
}

void ExportAsTXT(BillEntry* entryMap, const char* filePath){
    const char *data = GetEntryMapString(entryMap);
    FILE *fp = fopen(filePath, "w");
    if (fp)
    {
      fprintf(fp, "%s", data);
      fclose(fp);
    }
}

void ExportAsCSV(BillEntry* entryMap, const char* filePath){
  FILE* file = fopen(filePath, "w");

  int billCount = hmlen(entryMap);
  fprintf(file, "ID, Name, Frequency, Amount, Week, Fortnight, Month, Quarter, Year\n");

  for(int i = 0; i < billCount; i++){
    Bill bill = (entryMap)[i].value;
    uint64_t id = (entryMap)[i].key;
    double w, f, m, q, y;
    w = ConvertBillPaymentFrequency(&bill, WEEKLY);
    f = ConvertBillPaymentFrequency(&bill, FORTNIGHTLY);
    m = ConvertBillPaymentFrequency(&bill, MONTHLY);
    q = ConvertBillPaymentFrequency(&bill, QUARTERLY);
    y = ConvertBillPaymentFrequency(&bill, YEARLY);

    fprintf(file, "%lu, %s, %s, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n",
      id, bill.name, GetBillFreq(bill.frequency), bill.payment, w, f, m,
      q, y);
  }

  fclose(file);
}
