# Mojo-Budget

mojo-budget is a simple budget management program written in C for linux (and maybe windows).

## Features

- Add, edit and delete bills
- View bill payments by frequency (ie. weekly, fortnightly, montly ..etc)
- Saving and loading (as custom binary *.bud file) 
- Export budget as a text file
- Export to CSV (can then be opened as a spreadsheet)

## Dependancies

There are two programs, the gui version using imgui as the interface, or the terminal only version using ncurses.

- CMake (3.10 or higher)
- Ninja (optional, but my favourite)
- A C/C++ compiler (gcc/g++ or clang)

GUI:
- SDL2
- OpenGL 2.1
- cimgui
- tinyfiledialogs

Terminal:
- ncurses
- panel

Other deps used:
- stb (for stb-ds.h)

## Building

Most of the dependancies will be fetched automatically when building, but you will need to have the usual build tools and libraries for SDL2, OpenGL and ncurses installed.

Ubuntu:
'''bash
sudo apt update
sudo apt install build-essential cmake libsdl2-dev libgl1-mesa-dev libncurses-dev
'''

Arch Linux:
'''bash
sudo pacman -Sy base-devel cmake sdl2 mesa libgl ncurses
'''

'''bash
cmake -G Ninja -B build
cd build
ninja
'''

## Usage

'''bash
./mojo-budget-sdl # for the gui version
'''

or

'''bash
./mojo-budget-ncurses # for the terminal version
'''

## Notes

Instructions for use are pretty self explanatory when running the program, the terminal version uses keyboard shortcuts (ie.. a for add bill, e for edit, escape to quit ..etc), these shortcuts are displayed within the program. As for the gui version, you can load and save "bud" files from the file menu, aswell as export to either txt or csv.

Each bill has a name, frequency and amount. When you add an amount to a bill, thee program will calculate the amount you need to pay for each frequency (ie. if you add a bill with a frequency of "weekly" and an amount of 100, the program will calculate the fortnightly, monthly and yearly amounts for you). You can also edit the bill to change the name, frequency or amount.

# Author
This project was created by BadMojo.

## License

This project is licensed under the MIT License - see the LICENSE file for details.
