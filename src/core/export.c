#include "export.h"
#include "bill.h"
#include "utility.h"
#include "xlsxwriter.h"
#include <errno.h>
#include <stb/stb_ds.h>
#include <stdio.h>
#include <string.h>

#define BUD_MAGIC {0x1B, 'B', 'U', 'D'}
#define BUD_VERSION 1

typedef struct
{
  uint8_t magic[4];
  uint16_t version;
  uint8_t reserved[2];
} BudHeader;

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

  uint8_t expected_magic[4] = BUD_MAGIC;
  uint8_t peek[4];

  if (fread(peek, sizeof(peek), 1, file) != 1)
  {
    fprintf(stderr, "Error: failed to read from '%s'\n", filePath);
    fclose(file);
    return NULL;
  }
  rewind(file);

  // Free any existing income map before loading
  if (incomeMap)
  {
    hmfree(incomeMap);
    incomeMap = NULL;
    _nextIncomeID = 0;
  }

  if (memcmp(peek, expected_magic, sizeof(expected_magic)) == 0)
  {
    // Versioned file — read header first
    BudHeader header;
    if (fread(&header, sizeof(header), 1, file) != 1)
    {
      fprintf(stderr, "Error: failed to read header from '%s'\n", filePath);
      fclose(file);
      return NULL;
    }

    if (header.version == 1)
    {
      if (fread(budgetName, sizeof(budgetName), 1, file) != 1)
      {
        fprintf(stderr, "Error: failed to read budget name from '%s'\n", filePath);
        fclose(file);
        return NULL;
      }

      printf("Loading budget (v1): %s\n", trimmedPath);

      int billCount;
      if (fread(&billCount, sizeof(int), 1, file) != 1 || billCount < 0)
      {
        fprintf(stderr, "Error: failed to read bill count from '%s'\n", filePath);
        fclose(file);
        return NULL;
      }

      BillEntry *map = NULL;
      for (int i = 0; i < billCount; i++)
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
      }
      _nextID++;

      int incomeCount;
      if (fread(&incomeCount, sizeof(int), 1, file) != 1 || incomeCount < 0)
      {
        fprintf(stderr, "Error: failed to read income count from '%s'\n", filePath);
        hmfree(map);
        fclose(file);
        return NULL;
      }

      for (int i = 0; i < incomeCount; i++)
      {
        uint64_t key;
        Income income;
        if (fread(&key, sizeof(uint64_t), 1, file) != 1 ||
            fread(&income, sizeof(Income), 1, file) != 1)
        {
          fprintf(stderr, "Error: unexpected EOF reading income %d from '%s'\n", i, filePath);
          hmfree(map);
          fclose(file);
          return NULL;
        }
        hmput(incomeMap, key, income);
        _nextIncomeID++;
      }
      _nextIncomeID++;

      printf("Loaded %d bills, %d incomes.\n", billCount, incomeCount);
      fclose(file);
      return map;
    }
    else
    {
      fprintf(stderr, "Error: unsupported budget file version %u in '%s'\n",
              header.version, filePath);
      fclose(file);
      return NULL;
    }
  }
  else
  {
    // Legacy v0 format: budgetName + bills only, no header
    if (fread(budgetName, sizeof(budgetName), 1, file) != 1)
    {
      fprintf(stderr, "Error: failed to read budget name from '%s'\n", filePath);
      fclose(file);
      return NULL;
    }

    printf("Loading budget (legacy): %s\n", trimmedPath);

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
    }
    _nextID++;

    printf("Loaded %d bills (legacy format).\n", entryCount);
    fclose(file);
    return map;
  }
}

void BudgetSave(BillEntry *billMap, IncomeEntry *inMap, const char *filePath)
{
  const char *fileName = TrimPath(filePath);
  const char *trimmedPath = TrimHomePath(filePath);

  snprintf(budgetName, sizeof(budgetName), "%s", fileName);
  FILE *file = fopen(filePath, "wb");

  if (CheckFile(file, filePath, "save") != 0)
    return;

  SetSavePath(filePath);

  // Write versioned header
  BudHeader header;
  uint8_t magic[4] = BUD_MAGIC;
  memcpy(header.magic, magic, sizeof(header.magic));
  header.version = BUD_VERSION;
  header.reserved[0] = 0;
  header.reserved[1] = 0;
  fwrite(&header, sizeof(header), 1, file);

  fwrite(budgetName, sizeof(budgetName), 1, file);
  printf("Writing budget (v%u): %s\n", BUD_VERSION, trimmedPath);

  int billCount = hmlen(billMap);
  fwrite(&billCount, sizeof(int), 1, file);
  for (int i = 0; i < billCount; i++)
  {
    fwrite(&billMap[i].key, sizeof(uint64_t), 1, file);
    fwrite(&billMap[i].value, sizeof(Bill), 1, file);
  }

  int incomeCount = hmlen(inMap);
  fwrite(&incomeCount, sizeof(int), 1, file);
  for (int i = 0; i < incomeCount; i++)
  {
    fwrite(&inMap[i].key, sizeof(uint64_t), 1, file);
    fwrite(&inMap[i].value, sizeof(Income), 1, file);
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
  for (; *s; s++)
  {
    if (*s == '"')
      fputc('"', file);
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
    fprintf(file, ", $%s, $%.2f, $%.2f, $%.2f, $%.2f, $%.2f, $%.2f\n",
            GetBillFreq(bill.frequency), bill.payment, w, f, m, q, y);
  }

  fprintf(file, "\n, \"Total\", , , $%.2f, $%.2f, $%.2f, $%.2f, $%.2f\n",
          TotalBillsByFrequency(entryMap, WEEKLY),
          TotalBillsByFrequency(entryMap, FORTNIGHTLY),
          TotalBillsByFrequency(entryMap, MONTHLY),
          TotalBillsByFrequency(entryMap, QUARTERLY),
          TotalBillsByFrequency(entryMap, YEARLY));

  fclose(file);
}

void ExportAsTSV(BillEntry *entryMap, const char *filePath)
{
  FILE *file = fopen(filePath, "w");

  if (CheckFile(file, filePath, "export") != 0)
    return;

  int billCount = hmlen(entryMap);
  fprintf(
      file,
      "ID\tName\tFrequency\tAmount\tWeek\tFortnight\tMonth\tQuarter\tYear\n");

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

    fprintf(file, "%lu\t%s\t%s\t$%.2f\t$%.2f\t$%.2f\t$%.2f\t$%.2f\t$%.2f\n", id,
            bill.name, GetBillFreq(bill.frequency), bill.payment, w, f, m, q,
            y);
  }

  fprintf(file, "\n\tTotal\t\t$%.2f\t$%.2f\t$%.2f\t$%.2f\t$%.2f\n",
          TotalBillsByFrequency(entryMap, WEEKLY),
          TotalBillsByFrequency(entryMap, FORTNIGHTLY),
          TotalBillsByFrequency(entryMap, MONTHLY),
          TotalBillsByFrequency(entryMap, QUARTERLY),
          TotalBillsByFrequency(entryMap, YEARLY));

  fclose(file);
}

void ExportAsXLSX(BillEntry *entryMap, const char *filePath)
{
  int billCount = hmlen(entryMap);

  lxw_workbook *workbook = workbook_new(filePath);
  lxw_worksheet *worksheet = workbook_add_worksheet(workbook, "Budget");

  lxw_format *header_fmt = workbook_add_format(workbook);
  format_set_bold(header_fmt);

  lxw_format *currency_fmt = workbook_add_format(workbook);
  format_set_num_format(currency_fmt, "$#,##0.00");

  lxw_format *total_label_fmt = workbook_add_format(workbook);
  format_set_bold(total_label_fmt);

  lxw_format *total_currency_fmt = workbook_add_format(workbook);
  format_set_bold(total_currency_fmt);
  format_set_num_format(total_currency_fmt, "$#,##0.00");

  worksheet_set_column(worksheet, 0, 0, 6, NULL);
  worksheet_set_column(worksheet, 1, 1, 22, NULL);
  worksheet_set_column(worksheet, 2, 2, 14, NULL);
  worksheet_set_column(worksheet, 3, 3, 12, NULL);
  worksheet_set_column(worksheet, 4, 8, 14, NULL);
  worksheet_set_column(worksheet, 9, 9, 10, NULL);

  worksheet_write_string(worksheet, 0, 0, "ID", header_fmt);
  worksheet_write_string(worksheet, 0, 1, "Name", header_fmt);
  worksheet_write_string(worksheet, 0, 2, "Frequency", header_fmt);
  worksheet_write_string(worksheet, 0, 3, "Amount", header_fmt);
  worksheet_write_string(worksheet, 0, 4, "Weekly", header_fmt);
  worksheet_write_string(worksheet, 0, 5, "Fortnightly", header_fmt);
  worksheet_write_string(worksheet, 0, 6, "Monthly", header_fmt);
  worksheet_write_string(worksheet, 0, 7, "Quarterly", header_fmt);
  worksheet_write_string(worksheet, 0, 8, "Yearly", header_fmt);
  worksheet_write_string(worksheet, 0, 9, "Include", header_fmt);

  for (int i = 0; i < billCount; i++)
  {
    Bill bill = entryMap[i].value;
    uint64_t id = entryMap[i].key;
    int row = i + 1;

    // Excel formula row numbers are 1-based; API rows are 0-based.
    char freq_cell[16];
    char amount_cell[16];
    snprintf(freq_cell, sizeof(freq_cell), "C%d", row + 1);
    snprintf(amount_cell, sizeof(amount_cell), "D%d", row + 1);

    // Encodes ConvertBillPaymentFrequency's periodsPerYear lookup:
    // {Weekly=52, Fortnightly=26, Monthly=12, Quarterly=4, Yearly=1}
    char periods[256];
    snprintf(periods, sizeof(periods),
             "IF(%s=\"Weekly\",52,IF(%s=\"Fortnightly\",26,"
             "IF(%s=\"Monthly\",12,IF(%s=\"Quarterly\",4,1))))",
             freq_cell, freq_cell, freq_cell, freq_cell);

    char formula[512];

    worksheet_write_number(worksheet, row, 0, (double)id, NULL);
    worksheet_write_string(worksheet, row, 1, bill.name, NULL);
    worksheet_write_string(worksheet, row, 2, GetBillFreq(bill.frequency),
                           NULL);
    worksheet_write_number(worksheet, row, 3, bill.payment, currency_fmt);

    snprintf(formula, sizeof(formula), "=%s*%s/52", amount_cell, periods);
    worksheet_write_formula(worksheet, row, 4, formula, currency_fmt);

    snprintf(formula, sizeof(formula), "=%s*%s/26", amount_cell, periods);
    worksheet_write_formula(worksheet, row, 5, formula, currency_fmt);

    snprintf(formula, sizeof(formula), "=%s*%s/12", amount_cell, periods);
    worksheet_write_formula(worksheet, row, 6, formula, currency_fmt);

    snprintf(formula, sizeof(formula), "=%s*%s/4", amount_cell, periods);
    worksheet_write_formula(worksheet, row, 7, formula, currency_fmt);

    snprintf(formula, sizeof(formula), "=%s*%s", amount_cell, periods);
    worksheet_write_formula(worksheet, row, 8, formula, currency_fmt);

    worksheet_write_boolean(worksheet, row, 9, bill.include_in_totals ? 1 : 0,
                            NULL);
  }

  // Totals: SUMPRODUCT multiplies each frequency column by the Include (col J)
  // boolean so only included bills contribute, mirroring TotalBillsByFrequency.
  int totals_row = billCount + 1;
  worksheet_write_string(worksheet, totals_row, 1, "Total", total_label_fmt);

  const char *col_letters[] = {"E", "F", "G", "H", "I"};
  char sum_formula[128];
  for (int col = 4; col <= 8; col++)
  {
    const char *cl = col_letters[col - 4];
    snprintf(sum_formula, sizeof(sum_formula), "=SUMPRODUCT((J2:J%d)*%s2:%s%d)",
             billCount + 1, cl, cl, billCount + 1);
    worksheet_write_formula(worksheet, totals_row, col, sum_formula,
                            total_currency_fmt);
  }

  workbook_close(workbook);
}
