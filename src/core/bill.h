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
 
  typedef enum {
    INCOME_WEEKLY,
    INCOME_FORTNIGHTLY,
    INCOME_MONTHLY,
    INCOME_ALTERNATING_WEEKLY,
  } IncomeFrequency;

  typedef struct {
    char name[64];
    IncomeFrequency frequency;
    double amount;
    double amount_alt; // week-2 value, only used for INCOME_ALTERNATING_WEEKLY
    bool enable;
    bool locked;
  } Income;

  typedef struct {
    uint64_t key;
    Income value;
  } IncomeEntry;
  

  typedef enum { FILETYPE_TXT, FILETYPE_BUD } SaveFileType;
  extern char budgetName[MAX_BUDGET_NAME];
  extern BillEntry* entryMap;
  extern IncomeEntry* incomeMap;
  extern uint64_t _nextID;
  extern uint64_t _nextIncomeID;

  const char* ConvertDoubleToString(double value);
  void AddEntry(BillEntry** map, Bill entry);
  void RemoveEntry(BillEntry** map, uint64_t id);
  void ClearEntries(BillEntry** map);
  void AddIncomeEntry(IncomeEntry** map, Income entry);
  void RemoveIncomeEntry(IncomeEntry** map, uint64_t id);
  void ClearIncomeEntries(IncomeEntry** map);
  double GetIncomeAtFrequency(const Income* income, IncomeFrequency targetFreq);
  const char* GetIncomeFreqName(IncomeFrequency frequency);
  const char* GetBillFreq(PaymentFrequency frequency);
  double ConvertBillPaymentFrequency(const Bill* bill,
    PaymentFrequency targetFreq);
  double TotalBillsByFrequency(BillEntry* map, PaymentFrequency freq);
  const char* GetTotalPaymentsByFrequency(BillEntry* map);
  const char* GetIncomeMapString(IncomeEntry* map);
  const char* GetEntryMapString(BillEntry* map);
  void PrintEntryMap(BillEntry* map);
  // deprecated BillEntry* LoadEntryMap(const char* file);
  // deprecated void SaveEntryMap(const char* file, BillEntry* map, SaveFileType type);
#ifdef __cplusplus
}
#endif
#endif /* BILL_H */
