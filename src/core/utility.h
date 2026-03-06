#ifndef UTILITY_H
#define UTILITY_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

int CheckFile(FILE* file, const char* filePath, const char *func);
const char* TrimHomePath(const char* path);
const char* TrimPath(const char* path);
char* ConvertToCurrencyString(double amount);

#ifdef __cplusplus
}
#endif
#endif /* UTILITY_H */
