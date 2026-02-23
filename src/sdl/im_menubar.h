#ifndef IM_MENUBAR_H
#define IM_MENUBAR_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void DrawMenuBar(bool* running);
void DrawBudgetWindow();

void DrawUI(bool* running);

#ifdef __cplusplus
}
#endif
#endif /* IM_MENUBAR_H */
