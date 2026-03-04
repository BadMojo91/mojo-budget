#ifndef EXPORT_H
#define EXPORT_H

#include "bill.h"

#ifdef __cplusplus
extern "C" {
#endif

extern char savePath[4096];

void SetSavePath(const char* path);

BillEntry* BudgetLoad(const char* filePath);
void BudgetSave(BillEntry* entryMap, const char* filePath);

void ExportAsTXT(BillEntry* entryMap, const char* filePath);
void ExportAsCSV(BillEntry* entryMap, const char* filePath);


#ifdef __cplusplus
}
#endif
#endif /* EXPORT_H */
