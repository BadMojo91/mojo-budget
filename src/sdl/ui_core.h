#ifndef UI_CORE_H
#define UI_CORE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

  void DrawAppMenuBar(bool *running);
  void DrawBudgetWindow();
  void DrawCalendarWindow();

  void DrawUI(bool *running);

#ifdef __cplusplus
}
#endif
#endif /* UI_CORE_H */
