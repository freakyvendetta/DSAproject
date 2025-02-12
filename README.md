# 🕹️ C++ Terminal Pac-Man Arcade Simulation (DSA Project)

[![Language: C++](https://img.shields.io/badge/Language-C%2B%2B11%2B-blue.svg)](https://isocpp.org/)
[![Platform: Windows](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)](#)
[![Subject: Data Structures & Algorithms](https://img.shields.io/badge/Subject-Data%20Structures%20%26%20Algorithms-orange.svg)](#)

An interactive, real-time terminal-based **Pac-Man Arcade Simulation** developed in **C++** as a **Data Structures & Algorithms (DSA)** academic project. It features dynamic grid representation, algorithm-driven enemy AI pursuit logic (Manhattan distance heuristic), state management (invincibility timers & power-ups), non-blocking input handling, and colorful ANSI terminal rendering.

---

## 🚀 Key Features

* **2D Grid Spatial Data Structure**: 40×20 matrix representation of the game board handling collision detection for walls, pellets, power-ups, and entities.
* **Manhattan Distance Enemy AI Pursuit**: Demons dynamically calculate Manhattan distance ($\Delta x + \Delta y$) relative to Pac-Man's coordinates, switching between greedy aggressive tracking when within range and random exploration.
* **Non-Blocking Keyboard I/O**: Real-time continuous game loop using Win32 API (`conio.h` `_kbhit()`, `_getch()`, and `windows.h` `Beep()` sound effects).
* **State Management & Timers**: Dynamic invincibility power-up states (`P`), temporary ghost vulnerability/safe frames, active score tracking, and progressive level difficulty multipliers.
* **ANSI Color Rendering**: Multi-color terminal UI with ANSI escape sequences (Yellow Pac-Man, Red Demons, Green Food Pellets, Cyan Power-Ups).

---

## 🛠️ Data Structures & Algorithms Concepts Applied

1. **2D Matrix / Grid Graph Topology**: Used for environment bounds, path collision checks, and entity spatial mapping.
2. **Greedy Heuristic AI Pursuit**: Calculates shortest grid distance ($|x_1 - x_2| + |y_1 - y_2|$) for real-time demon pathing towards player coordinates.
3. **State Machines & Game Loop Synchronization**: Discrete finite state logic (`NORMAL`, `INVINCIBLE`, `PAUSED`, `GAME OVER`) with frame timing synchronization via `Sleep()`.
4. **Pseudo-Random Map Generation**: Algorithmic obstacle distribution and dynamic food pellet positioning ensuring randomized board setups per level.

---

## 💻 Compilation & Setup Instructions

### Prerequisites
* **OS**: Windows (uses `<conio.h>` & `<windows.h>`)
* **Compiler**: MinGW `g++` or Microsoft Visual C++ (`cl`)

### Build & Run Commands

```powershell
# 1. Clone repository
git clone https://github.com/freakyvendetta/DSAproject.git
cd DSAproject

# 2. Compile using MinGW g++
g++ -o pacman.exe pacmansimulation.cpp

# 3. Launch game
.\pacman.exe
```

---

## 🎮 Game Controls

| Key | Action |
|---|---|
| `W` | Move Up |
| `S` | Move Down |
| `A` | Move Left |
| `D` | Move Right |
| `P` | Pause / Resume Game |
| `Q` | Quit Game |

---

## 📄 Academic Acknowledgment
Developed as a semester project for **Data Structures & Algorithms (DSA)** course.
