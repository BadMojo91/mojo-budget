#ifndef BILL_H
#define BILL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

  typedef enum {
    WEEKLY,
    FORTNIGHTLY,
    MONTHLY,
    QUARTERLY,
    YEARLY
  } PaymentFrequency;

  typedef struct {
    char name[256];
    PaymentFrequency frequency;
    double payment;
  } Bill;

  typedef struct {
    uint64_t key;
    Bill value;
  } BillEntry;

  typedef enum { FILETYPE_TXT, FILETYPE_BUD } SaveFileType;

  extern BillEntry* entryMap;
  static uint64_t _nextID = 0;

  void AddEntry(BillEntry** map, Bill entry);
  void RemoveEntry(BillEntry** map, uint64_t id);
  const char* GetBillFreq(PaymentFrequency frequency);
  double ConvertBillPaymentFrequency(const Bill* bill,
    PaymentFrequency targetFreq);
  const char* GetTotalPaymentsByFrequency(BillEntry* map);
  const char* GetEntryMapString(BillEntry* map);
  void PrintEntryMap(BillEntry* map);
  BillEntry* LoadEntryMap(const char* file);
  void SaveEntryMap(const char* file, BillEntry* map, SaveFileType type);
#ifdef __cplusplus
}
#endif
#endif /* BILL_H */
