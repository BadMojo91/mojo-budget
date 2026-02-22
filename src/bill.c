#define STB_DS_IMPLEMENTATION 
#include <stb/stb_ds.h>
#include <stddef.h>
#include <stdio.h>
#include "bill.h"

BillEntry *entryMap = NULL;

void AddEntry(BillEntry **map, Bill billEntry) {
  uint64_t newID = _nextID++;
  hmput(*map, newID, billEntry);
  printf("Adding bill entry: %s\nID: %lu\n", billEntry.name, newID);

}

const char *GetBillFreq(PaymentFrequency frequency) {
  switch (frequency) {
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
                                   PaymentFrequency targetFreq) {
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

const char *GetTotalPaymentsByFrequency(BillEntry *map) {
  static char result[512];
  double totals[5] = {0}; // WEEKLY, FORTNIGHTLY, MONTHLY, QUARTERLY, YEARLY
  int entryCount = hmlen(map);

  for (int i = 0; i < entryCount; i++) {
    Bill bill = map[i].value;
    for (int freq = WEEKLY; freq <= YEARLY; freq++) {
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

void PrintEntryMap(BillEntry *map) {
  int entryCount = hmlen(map);
  printf("Found entries: %d\n", entryCount);
  printf("\n\n");
  printf("---------------------------------------------------\n");
  printf("-                      BILLS                      -\n");
  printf("---------------------------------------------------\n\n");
  printf("ID | Name               | Frequency      | Amount    | Week      | Fortnight   | Month     | Quarter   | Year      |\n");
  printf("---------------------------------------------------------------------------------------------------------------\n");
  for (int i = 0; i < entryCount; i++) {
    Bill bill = map[i].value;
    uint64_t id = map[i].key;
    double w,f,m,q,y;
    w = ConvertBillPaymentFrequency(&bill, WEEKLY);
    f = ConvertBillPaymentFrequency(&bill, FORTNIGHTLY);
    m = ConvertBillPaymentFrequency(&bill, MONTHLY);
    q = ConvertBillPaymentFrequency(&bill, QUARTERLY);
    y = ConvertBillPaymentFrequency(&bill, YEARLY);

    printf("%-2lu | %-18s | %-14s | $%8.2f | $%8.2f | $%10.2f | $%8.2f | $%8.2f | $%8.2f |\n",
           id,
           bill.name,
           GetBillFreq(bill.frequency),
           bill.payment,
           w,
           f,
           m,
           q,
           y);
  }
  printf("\n\n---------------------------------------------------\n");
  printf("%s", GetTotalPaymentsByFrequency(map));
  printf("---------------------------------------------------\n\n");
}
