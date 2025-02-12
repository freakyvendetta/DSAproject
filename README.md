# 🕹️ C++ Terminal Pac-Man Arcade Simulation (`pacman-dsa-cpp`)

[![Language: C++](https://img.shields.io/badge/Language-C%2B%2B11%2B-blue.svg)](https://isocpp.org/)
[![Platform: Windows](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)](#)
[![Subject: Data Structures & Algorithms](https://img.shields.io/badge/Subject-Data%20Structures%20%26%20Algorithms-orange.svg)](#)

An interactive, real-time terminal-based **Pac-Man Arcade Simulation** developed in **C++** as a **Data Structures & Algorithms (DSA)** academic project. It features dynamic grid representation, Breadth-First Search (BFS) enemy AI pathfinding, real-time wall-clock timer synchronization, dual WASD & Arrow Key controls, an interactive difficulty menu, and dynamic console auto-centering.

---

## 🚀 Key Features

* **2D Grid Spatial Data Structure**: 40×20 matrix representation of the game board handling $O(1)$ collision detection for walls, pellets, power-ups, and entities.
* **Breadth-First Search (BFS) AI Pathfinding**: Ghosts run a queue-based BFS graph traversal algorithm (`getNextBfsStep`) to compute the exact shortest path through maze walls towards Pac-Man.
* **Interactive Difficulty Selection Menu**: Choose between `Easy`, `Medium`, and `Hard` modes at startup, dynamically configuring ghost velocities, initial ghost counts, and invincibility power-up durations.
* **Dynamic Terminal Auto-Centering**: Real-time Win32 console buffer measurement (`GetConsoleScreenBufferInfo`) automatically centers the retro arcade header, game board, score HUD, and Game Over screen in any terminal window size.
* **Real-Time Wall-Clock Timers**: Millisecond-accurate invincibility timers (`GetTickCount()`) scaling duration progressively per level transition.
* **Non-Blocking Keyboard I/O**: Real-time continuous game loop using Win32 API (`conio.h` `_kbhit()`, `_getch()`, and `windows.h` `Beep()` audio cues).
* **Flicker-Free 60 FPS Rendering**: In-place console cursor repositioning (`SetConsoleCursorPosition`) replacing `system("cls")` for smooth visual rendering.
* **Input Buffer Safety**: Automatic keyboard buffer flushing (`FlushConsoleInputBuffer`) preventing stuck keypresses from spilling into menus.

---

## 🛠️ Data Structures & Algorithms Concepts Applied

1. **Breadth-First Search (BFS Graph Traversal)**: Queue-based (`std::queue`) shortest path search algorithm operating on the 2D grid graph to track Pac-Man.
2. **2D Matrix / Grid Topology**: Used for environment bounds, path collision checks, and entity spatial mapping.
3. **Random Shuffling Algorithms**: `std::random_shuffle` over empty cell vectors to scatter 140 food pellets evenly across the full map.
4. **State Machine & Game Loop Synchronization**: Finite state machine handling game states (`MENU`, `PLAYING`, `PAUSED`, `LEVEL_CLEARED`, `GAME_OVER`).
5. **Tile Preservation Pattern**: Ghosts preserve underlying grid tiles (`underTile`) when moving, restoring pellets/power-ups when leaving.

---

## 🎮 Game Controls

| Control | Key Input | Action |
|---|---|---|
| **Move Up** | `W` or `Up Arrow` | Move Pac-Man Up |
| **Move Down** | `S` or `Down Arrow` | Move Pac-Man Down |
| **Move Left** | `A` or `Left Arrow` | Move Pac-Man Left |
| **Move Right** | `D` or `Right Arrow` | Move Pac-Man Right |
| **Pause / Resume** | `SPACE` or `P` | Pause game (or press any **Move Key** to Resume & Move!) |
| **Quit** | `Q` | Exit Game |
| **Difficulty Select** | `1`, `2`, `3`, or `Q` | Select `[1] Easy`, `[2] Medium`, `[3] Hard`, or `[Q] Quit` on startup menu |

---

## 💻 Compilation & Setup Instructions

### Prerequisites
* **OS**: Windows (uses `<conio.h>` & `<windows.h>`)
* **Compiler**: MinGW `g++` or Microsoft Visual C++ (`cl`)

### Build & Run Commands

```powershell
# 1. Clone repository
git clone https://github.com/freakyvendetta/pacman-dsa-cpp.git
cd pacman-dsa-cpp

# 2. Option A: Compile using MinGW g++
g++ -O2 -std=c++11 -o pacman.exe pacmansimulation.cpp

# 2. Option B: Automated build script
.\build.bat

# 3. Launch game
.\pacman.exe
```

---

## 📄 Academic Acknowledgment
Developed as a semester project for **Data Structures & Algorithms (DSA)** course.
