#include "utility.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Anything other than zero is an error
// eg. if(CheckFile(file, filePath, "export") != 0) return;
int CheckFile(FILE *file, const char *filePath, const char *func)
{
  if (!file)
  {
    fprintf(stderr, "Could not %s file: %s\n%s\n", func, filePath,
            strerror(errno));
    return errno;
  }
  return 0;
}

const char *TrimHomePath(const char *path)
{
  static char buf[4096];
  const char *home = getenv("HOME");

  if (!path)
    return NULL;
  if (!home || home[0] == '\0')
    return path;

  size_t homeLen = strlen(home);
  if (strncmp(path, home, homeLen) == 0 &&
      (path[homeLen] == '/' || path[homeLen] == '\\' || path[homeLen] == '\0'))
  {
    snprintf(buf, sizeof(buf), "~%s", path + homeLen);
    return buf;
  }

  return path;
}

const char *TrimPath(const char *path)
{
  const char *lastSlash = strrchr(path, '/');
  if (!lastSlash)
  {
    lastSlash = strrchr(path, '\\');
  }
  return lastSlash ? lastSlash + 1 : path;
}

const char *TrimExt(const char *fileName)
{
  const char *extension = strrchr(fileName, '.');

  if (!extension)
  {
    return fileName;
  }

  size_t nameLen = extension - fileName;
  static char trimmedName[256];
  snprintf(trimmedName, sizeof(trimmedName), "%.*s", (int)nameLen, fileName);
  return trimmedName;
}

const char* GetExt(const char* fileName){
    const char* ext = strrchr(fileName, '.');
     if(!ext)
      return NULL;
    return ext;
}

const char* SetExt(const char* fileName, const char* ext){
  static char result[4096];
  const char* dot = strrchr(fileName, '.');
  size_t baseLen;
  if(dot)
  {
    baseLen = dot - fileName;
  }
  else {
    baseLen = strlen(fileName);
  }
  snprintf(result, sizeof(result), "%.*s.%s", (int)baseLen, fileName, ext);
  return result;
}

char *ConvertToCurrencyString(double amount)
{
  static char result[64];

  if (amount < 0)
  {
    snprintf(result, sizeof(result), "-$%.2f", -amount);
  }
  else if (amount == 0)
  {
    snprintf(result, sizeof(result), "$0.00");
  }
  else if (amount > 0 && amount < 0.01)
  {
    snprintf(result, sizeof(result), "$%.4f", amount);
  }
  else if (amount >= 1000000)
  {
    snprintf(result, sizeof(result), "$%.2fM", amount / 1000000);
  }
  else if (amount >= 1000)
  {
    snprintf(result, sizeof(result), "$%.2fK", amount / 1000);
  }
  else
  {
    snprintf(result, sizeof(result), "$%.2f", amount);
  }

  return result;
}
