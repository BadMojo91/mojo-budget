#ifndef UTILITY_H
#define UTILITY_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

int CheckFile(FILE* file, const char* filePath, const char *func);
const char* TrimHomePath(const char* path);
const char* TrimPath(const char* path);
const char* TrimExt(const char* fileName);
const char* GetExt(const char* fileName);
const char* SetExt(const char* fileName, const char* ext);
char* ConvertToCurrencyString(double amount);

#ifdef __cplusplus
}
#endif
#endif /* UTILITY_H */
