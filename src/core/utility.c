#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
