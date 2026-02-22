#include <stdint.h>
#include <stdio.h>

#define STB_DS_IMPLEMENTATION
#include <stb/stb_ds.h>

typedef enum { WEEKLY, FORTNIGHTLY, MONTHLY, QUARTERLY, YEARLY } PaymentFrequency;

typedef struct {
  char name[256];
  PaymentFrequency frequency;
  double payment;
} Bill;

typedef struct {
  uint64_t key;
  Bill value;
} BillEntry;

BillEntry *entryMap = NULL;
uint64_t _nextID = 0;

void AddEntry(Bill billEntry) {
  uint64_t newID = _nextID++;
  hmput(entryMap, newID, billEntry);
  printf("Adding bill entry: %s\n", billEntry.name);
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

double ConvertBillPaymentFrequency(const Bill *bill, PaymentFrequency targetFreq) {
  // Approximate periods per year for each frequency
  static const double periodsPerYear[] = {
    52.0,   // WEEKLY
    26.0,   // FORTNIGHTLY
    12.0,   // MONTHLY
    4.0,    // QUARTERLY
    1.0     // YEARLY
  };

  double billPeriods = periodsPerYear[bill->frequency];
  double targetPeriods = periodsPerYear[targetFreq];
  double annualAmount = bill->payment * billPeriods;
  return annualAmount / targetPeriods;
}

const char* GetTotalPaymentsByFrequency(BillEntry *map) {
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
    "Weekly: $%.2f\nFortnightly: $%.2f\nMonthly: $%.2f\nQuarterly: $%.2f\nYearly: $%.2f\n",
    totals[WEEKLY], totals[FORTNIGHTLY], totals[MONTHLY], totals[QUARTERLY], totals[YEARLY]
  );


  return result;
}

void PrintEntryMap(BillEntry *map) {
  int entryCount = hmlen(map);
  printf("\n\n");
  printf("---------------------------------------------------\n");
  printf("-                      BILLS                      -\n");
  printf("---------------------------------------------------\n\n");
  for (int i = 0; i < entryCount; i++) {
    Bill bill = map[i].value;
    printf("%s        %s        $%.2f\n", bill.name, GetBillFreq(bill.frequency), bill.payment);
  }
  printf("\n\n---------------------------------------------------\n");
  printf("%s", GetTotalPaymentsByFrequency(map));
  printf("---------------------------------------------------\n\n");
}

int main() {
  Bill bill1 = {"Electricity", MONTHLY, 100.0};
  Bill bill2 = {"Water", MONTHLY, 50.0};
  Bill bill3 = {"Internet", MONTHLY, 75.0};

  AddEntry(bill1);
  AddEntry(bill2);
  AddEntry(bill3);

  PrintEntryMap(entryMap);
  return 0;
}
