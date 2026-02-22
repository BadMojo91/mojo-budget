#include "bill.h"

int main() {
  Bill bill1 = {"Electricity", MONTHLY, 100.0};
  Bill bill2 = {"Water", MONTHLY, 50.0};
  Bill bill3 = {"Internet", MONTHLY, 75.0};

  AddEntry(&entryMap, bill1);
  AddEntry(&entryMap, bill2);
  AddEntry(&entryMap, bill3);

  PrintEntryMap(entryMap);
  return 0;
}
