# Snake Game in C (ncurses)

A classic Snake game implemented in C using the ncurses library.  
The game runs on a **256×256** grid, uses `+` for the border and `#` for both snake body and food.

## Features

- Keyboard arrow keys (↑ ↓ ← →) to control direction
- Non‑blocking input with smooth 100 ms per move
- Food is randomly placed, avoiding the snake body
- Score display and game‑over screen
- Full Linux kernel coding style

## Requirements

- Linux / macOS / any system with **ncurses** library
- GCC (or any C89/C99 compiler)

### Install ncurses (if missing)

**Debian/Ubuntu:**  
`sudo apt install libncurses-dev`

**Red Hat/Fedora:**  
`sudo dnf install ncurses-devel`

**macOS (Homebrew):**  
`brew install ncurses`

## Compilation

```bash
make          # release build (optimized)
make debug    # debug build with symbols