#ifndef BILL_H
#define BILL_H


#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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

extern BillEntry *entryMap;
static uint64_t _nextID = 0;

void AddEntry(BillEntry** map, Bill entry);

const char* GetBillFreq(PaymentFrequency frequency);
double ConvertBillPaymentFreqeuncy(const Bill* bill, PaymentFrequency targetFreq);
const char* GetTotalPaymentsByFrequency(BillEntry *map);
void PrintEntryMap(BillEntry *map);

#ifdef __cplusplus
}
#endif
#endif /* BILL_H */
