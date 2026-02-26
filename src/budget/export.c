#include "export.h"
#include <stdio.h>

#include <stb/stb_ds.h>

void ExportAsCSV(BillEntry** billEntryMap, const char* filePath){
  FILE* file = fopen(filePath, "w");

  int billCount = hmlen(*billEntryMap);
  fprintf(file, "ID, Name, Frequency, Amount, Week, Fortnight, Month, Quarter, Year\n");

  for(int i = 0; i < billCount; i++){
    Bill bill = (*billEntryMap)[i].value;
    uint64_t id = (*billEntryMap)[i].key;
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
