#ifndef BILL_H
#define BILL_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

  #define MAX_BUDGET_NAME 256
  #define MAX_BILL_NAME 256

  typedef enum {
    WEEKLY,
    FORTNIGHTLY,
    MONTHLY,
    QUARTERLY,
    YEARLY
  } PaymentFrequency;

  typedef struct {
    char name[MAX_BILL_NAME];
    PaymentFrequency frequency;
    double payment;
    bool include_in_totals;
    bool locked;
  } Bill;

  typedef struct {
    uint64_t key;
    Bill value;
  } BillEntry;
 
  typedef struct {
    char name[64];
    double income;
    bool enable;
  } Income;

  typedef struct {
    uint64_t key;
    Income value;
  } IncomeEntry; 
  

  typedef enum { FILETYPE_TXT, FILETYPE_BUD } SaveFileType;
  extern char budgetName[MAX_BUDGET_NAME];
  extern BillEntry* entryMap;
  extern uint64_t _nextID;

  void AddEntry(BillEntry** map, Bill entry);
  void RemoveEntry(BillEntry** map, uint64_t id);
  void ClearEntries(BillEntry** map);
  const char* GetBillFreq(PaymentFrequency frequency);
  double ConvertBillPaymentFrequency(const Bill* bill,
    PaymentFrequency targetFreq);
  const char* GetTotalPaymentsByFrequency(BillEntry* map);
  const char* GetEntryMapString(BillEntry* map);
  void PrintEntryMap(BillEntry* map);
  // deprecated BillEntry* LoadEntryMap(const char* file);
  // deprecated void SaveEntryMap(const char* file, BillEntry* map, SaveFileType type);
#ifdef __cplusplus
}
#endif
#endif /* BILL_H */
