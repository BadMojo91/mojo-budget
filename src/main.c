#include "bill.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

static void draw_entry_map(WINDOW *win, BillEntry *map) {
  werase(win);
  box(win, 0, 0);
  mvwprintw(win, 1, 2, "Bills:");

  const char *map_str = GetEntryMapString(map);
  int row = 3;
  const char *line = map_str;
  while (*line) {
    const char *eol = strchr(line, '\n');
    int len;
    if (eol) {
      len = eol - line;
    } else {
      len = strlen(line);
    }
    if (row >= getmaxy(win) - 1) break;
    mvwaddnstr(win, row, 2, line, len);
    row++;
    if (eol) {
      line = eol + 1;
    } else {
      break;
    }
  }
  wrefresh(win);
}

static void draw_keymap(WINDOW *win) {
  werase(win);
  box(win, 0, 0);
  mvwprintw(win, 1, 2, "Keymap:");
  mvwprintw(win, 3, 2, "a: Add Entry");
  mvwprintw(win, 4, 2, "r: Remove Entry");
  mvwprintw(win, 5, 2, "l: Load Entry Map");
  mvwprintw(win, 6, 2, "s: Save Entry Map");
  mvwprintw(win, 7, 2, "e: Exit");
  wrefresh(win);
}

static void refresh_all(WINDOW *entry_win, WINDOW *keymap_win, BillEntry *map) {
  draw_entry_map(entry_win, map);
  draw_keymap(keymap_win);
  touchwin(entry_win);
  touchwin(keymap_win);
  wrefresh(entry_win);
  wrefresh(keymap_win);
}

int main() {
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);
  refresh();

  int entry_win_height = 30;
  int entry_win_width = 120;
  int keymap_win_height = 10;
  int keymap_win_width = 30;

  WINDOW *entry_win = newwin(entry_win_height, entry_win_width, 0, 0);
  WINDOW *keymap_win = newwin(keymap_win_height, keymap_win_width, 0, entry_win_width + 2);

  BillEntry *entryMap = NULL;

  int running = 1;
  while (running) {
    refresh_all(entry_win, keymap_win, entryMap);

    int ch = getch();
    if (ch == 'a') {
      WINDOW *add_win = newwin(15, 60, 5, 5);
      keypad(add_win, TRUE);
      box(add_win, 0, 0);
      mvwprintw(add_win, 1, 2, "Add Bill Entry");

      char name[256] = {0};
      char payment_str[64] = {0};
      int freq = 2;

      curs_set(1);
      echo();

      mvwprintw(add_win, 3, 2, "Name: ");
      wrefresh(add_win);
      mvwgetnstr(add_win, 3, 8, name, 255);

      mvwprintw(add_win, 5, 2, "Payment: ");
      wrefresh(add_win);
      mvwgetnstr(add_win, 5, 11, payment_str, 63);

      const char *freq_labels[] = {"WEEKLY", "FORTNIGHTLY", "MONTHLY", "QUARTERLY", "YEARLY"};
      int freq_count = 5;
      int selected = 2;

      noecho();
      curs_set(0);

      int choosing = 1;
      while (choosing) {
        mvwprintw(add_win, 7, 2, "Frequency (UP/DOWN, ENTER):     ");
        for (int i = 0; i < freq_count; i++) {
          if (i == selected) {
            wattron(add_win, A_REVERSE);
          }
          mvwprintw(add_win, 8 + i, 4, "%-15s", freq_labels[i]);
          if (i == selected) {
            wattroff(add_win, A_REVERSE);
          }
        }
        wrefresh(add_win);

        int key = wgetch(add_win);
        if (key == KEY_UP && selected > 0) {
          selected--;
        } else if (key == KEY_DOWN && selected < freq_count - 1) {
          selected++;
        } else if (key == '\n' || key == KEY_ENTER) {
          choosing = 0;
        }
      }
      freq = selected;

      double payment = atof(payment_str);

      Bill bill = {0};
      strncpy(bill.name, name, 255);
      bill.payment = payment;
      bill.frequency = freq;
      AddEntry(&entryMap, bill);

      delwin(add_win);
    } else if (ch == 'r') {
      WINDOW *rem_win = newwin(5, 40, 7, 7);
      box(rem_win, 0, 0);
      mvwprintw(rem_win, 1, 2, "Remove Bill Entry");
      mvwprintw(rem_win, 2, 2, "ID: ");
      wrefresh(rem_win);

      char id_str[32] = {0};
      curs_set(1);
      echo();
      mvwgetnstr(rem_win, 2, 6, id_str, 31);
      noecho();
      curs_set(0);

      uint64_t id = strtoull(id_str, NULL, 10);
      RemoveEntry(&entryMap, id);

      delwin(rem_win);
    } else if (ch == 'l') {
      WINDOW *load_win = newwin(5, 50, 7, 7);
      box(load_win, 0, 0);
      mvwprintw(load_win, 1, 2, "Load Entry Map");
      mvwprintw(load_win, 2, 2, "Filepath: ");
      wrefresh(load_win);

      char filepath[256] = {0};
      curs_set(1);
      echo();
      mvwgetnstr(load_win, 2, 12, filepath, 255);
      noecho();
      curs_set(0);

      BillEntry *loaded = LoadEntryMap(filepath);
      if (loaded) {
        entryMap = loaded;
      }

      delwin(load_win);
    } else if (ch == 's') {
      WINDOW *save_win = newwin(7, 50, 7, 7);
      box(save_win, 0, 0);
      mvwprintw(save_win, 1, 2, "Save Entry Map");
      mvwprintw(save_win, 2, 2, "Filepath: ");
      wrefresh(save_win);

      char filepath[256] = {0};
      curs_set(1);
      echo();
      mvwgetnstr(save_win, 2, 12, filepath, 255);
      noecho();

      const char *type_labels[] = {"TXT", "BUD"};
      int selected = 0;
      curs_set(0);

      int choosing = 1;
      while (choosing) {
        mvwprintw(save_win, 4, 2, "Type (UP/DOWN, ENTER):");
        for (int i = 0; i < 2; i++) {
          if (i == selected) {
            wattron(save_win, A_REVERSE);
          }
          mvwprintw(save_win, 5 + i, 4, "%-10s", type_labels[i]);
          if (i == selected) {
            wattroff(save_win, A_REVERSE);
          }
        }
        wrefresh(save_win);

        int key = wgetch(save_win);
        if (key == KEY_UP && selected > 0) {
          selected--;
        } else if (key == KEY_DOWN && selected < 1) {
          selected++;
        } else if (key == '\n' || key == KEY_ENTER) {
          choosing = 0;
        }
      }

      SaveEntryMap(filepath, entryMap, selected == 0 ? FILETYPE_TXT : FILETYPE_BUD);
      delwin(save_win);
    } else if (ch == 'e') {
      running = 0;
    }
  }

  delwin(entry_win);
  delwin(keymap_win);
  endwin();
  return 0;
}
