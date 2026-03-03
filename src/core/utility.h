#ifndef UTILITY_H
#define UTILITY_H



#ifdef __cplusplus
extern "C" {
#endif

const char* TrimHomePath(const char* path);
const char* TrimPath(const char* path);
char* ConvertToCurrencyString(double amount);

#ifdef __cplusplus
}
#endif
#endif /* UTILITY_H */
