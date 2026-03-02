#ifndef EXPORT_H
#define EXPORT_H

#include "bill.h"

#ifdef __cplusplus
extern "C" {
#endif

void ExportAsTXT(BillEntry** billEntryMap, const char* filePath);
void ExportAsCSV(BillEntry** billEntryMap, const char* filePath);


#ifdef __cplusplus
}
#endif
#endif /* EXPORT_H */
