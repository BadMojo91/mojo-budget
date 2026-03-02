#include "utility.h"
#include <stdio.h>

char* ConvertToCurrencyString(double amount)
{
  static char result[64];

  if(amount < 0)
  {
    snprintf(result, sizeof(result), "-$%.2f", -amount);
  }
  else if(amount == 0)
  {
    snprintf(result, sizeof(result), "$0.00");
  }
  else if(amount > 0 && amount < 0.01)
  {
    snprintf(result, sizeof(result), "$%.4f", amount);
  }
  else if(amount >= 1000000)
  {
    snprintf(result, sizeof(result), "$%.2fM", amount / 1000000);
  }
  else if(amount >= 1000)
  {
    snprintf(result, sizeof(result), "$%.2fK", amount / 1000);
  }
  else
  {
    snprintf(result, sizeof(result), "$%.2f", amount);
  }

  return result;
}
