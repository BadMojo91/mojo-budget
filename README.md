# Mojo-Budget

A personal budget management tool written in C. Comes in two flavours: a graphical interface built on SDL2 + Dear ImGui, and a terminal interface using ncurses.

---

## Features

### Bills
- Add, edit and delete bills with a **name**, **payment frequency** and **amount**
- Frequencies: Weekly, Fortnightly, Monthly, Quarterly, Yearly
- Automatically calculates equivalent amounts across all frequencies
- **Lock** individual bills to prevent accidental edits
- Toggle bills on/off to include or exclude them from totals
- Set a **last bill date** per bill and view the estimated **next due date** (calculated from frequency)
- Assign a **custom highlight colour** per bill for the calendar (colour picker, random colour, copy/paste between bills)

### Income
- Add, edit and delete income sources with a **name**, **frequency** and **amount**
- Frequencies: Weekly, Fortnightly, Monthly, **Alternating Weekly** (different amounts for odd/even weeks)
- Automatically calculates Weekly, Fortnightly, Monthly, Quarterly and Yearly equivalents
- Lock and enable/disable individual income entries

### Totals
- Summary table showing **Income**, **Expenses** and **Net** (income minus expenses) across all frequencies
- Only enabled bills and incomes are counted

### Yearly Calendar *(GUI only)*
- Toggle via **View → Calendar** — opens as a side panel
- 12-month grid with each month showing day numbers aligned to weekdays (Mon–Sun)
- Days with a bill due are **highlighted in that bill's assigned colour**
- Hover over a highlighted day to see a tooltip listing every bill due on that date, its amount, and a combined total if multiple bills fall on the same day

### File Management
- Save and load budgets as binary **`.bud`** files (File menu)
- File format is versioned — v1/v2/v3 files all load correctly with automatic migration
- **Export as TXT** — formatted text report with budget name title, income table, expenses table and totals
- **Export as CSV** — spreadsheet-compatible export

---

## Dependencies

The GUI version requires SDL2, OpenGL and Dear ImGui. The terminal version requires ncurses. Most dependencies are fetched automatically at build time.

### System packages

**Ubuntu / Debian:**
```bash
sudo apt update
sudo apt install build-essential cmake ninja-build libsdl2-dev libgl1-mesa-dev libncurses-dev
```

**Arch Linux:**
```bash
sudo pacman -Sy base-devel cmake ninja sdl2 mesa libgl ncurses
```

**Automatically fetched at build time:**
- [cimgui](https://github.com/cimgui/cimgui) (Dear ImGui C bindings)
- [tinyfiledialogs](https://sourceforge.net/projects/tinyfiledialogs/)
- [stb](https://github.com/nothings/stb) (`stb_ds.h` hash map)
- [libxlsxwriter](https://libxlsxwriter.github.io/) (XLSX export)
- zlib

---

## Building

Use the provided scripts from the project root. Build output goes into `build/linux` or `build/win64`.

### Linux
```bash
./build-linux.sh
```

### Windows (cross-compile via MinGW-w64)
```bash
./build-windows.sh
```
Requires `mingw-w64` to be installed. Uses the bundled `toolchain-mingw64.cmake`. Note: the ncurses frontend is excluded from the Windows build.

### Clean
```bash
./clean.sh
```

### Manual (CMake)
```bash
cmake -G Ninja -B build/linux
cmake --build build/linux
```

---

## Usage

### GUI (SDL2 + Dear ImGui)
```bash
./build/linux/mojo-budget-sdl
```

- **File menu** — New, Open, Save, Save As, Export TXT, Export CSV
- **View menu** — toggle the yearly calendar panel
- **Income table** — top section; Add button bottom-right
- **Bills table** — middle section; Add button bottom-right
- **Totals table** — bottom section; shows Income / Expenses / Net by frequency
- Drag the resize grips between tables to adjust their heights

### Terminal (ncurses)
```bash
./build/linux/mojo-budget-ncurses
```

Keyboard shortcuts are displayed within the program (e.g. `a` to add, `e` to edit, `Esc` to quit).

---

## File Format

Budget files use the `.bud` extension (custom binary format). The current format version is **3**.

| Version | Notes |
|---------|-------|
| v3 | Adds per-bill RGB calendar highlight colour |
| v2 | Adds bill last-date fields for due-date tracking |
| v1 | Adds income entries and versioned header |
| v0 (legacy) | Bills only, no header |

Older files are automatically migrated when opened.

---

## Author

Created by [BadMojo](https://github.com/badmojo).

## License

MIT License — see [LICENSE](LICENSE) for details.

