#include "budget/bill.h"
#include <ncurses.h>
#include <panel.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[]) {
  if(argc > 1) {
    entryMap = LoadEntryMap(argv[1]);
    if(!entryMap){
      printf("Failed to load file: %s\n", argv[1]);
      return 1;
    }
  }

  WINDOW *mainWindow;
  PANEL *mainPanel;

  int lines = 0, cols = 40, y = 2, x = 4, i;

  initscr();
  cbreak();
  noecho();
  
  mainWindow = newwin(lines, cols, y, x);
  box(mainWindow, 0, 0);

  mainPanel = new_panel(mainWindow);
  
  update_panels();
  doupdate();

  getch();

  endwin();
}
