#include <iostream>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include <windows.h>
#include <vector>
#include <queue>
#include <algorithm>

#define WIDTH 40
#define HEIGHT 20
#define PACMAN 'C'
#define WALL '#'
#define FOOD '.'
#define EMPTY ' '
#define DEMON 'X'
#define POWERUP 'P'
#define INVINCIBILITY_DURATION_MS 10000  // 10 real seconds
#define TARGET_FOOD_COUNT 140            // Rich food density across maze

struct Demon {
    int y, x;
    char underTile;
};

struct Point {
    int y, x;
};

int res = 0;            // 0: running, 1: game over, 2: level cleared
int score = 0;
int highScore = 0;
int pacman_x, pacman_y;
char board[HEIGHT][WIDTH];
int food = 0;
bool invincible = false;
DWORD invincibleEndTime = 0;
int invincibleTimeLeft = 0;
bool paused = false;
int level = 1; 
int demonSpeed = 350;
std::vector<Demon> demons;

// Function Prototypes
void initialize(int currentLevel);
void draw();
void movePacman(int move_x, int move_y);
void moveDemons();
void placePowerUps();
void placeFood();
void showGameOver(bool win);
void pauseGame();
void hideCursor();
void setCursorPosition(int x, int y);
void flushInputBuffer();

// Hide terminal cursor for flicker-free rendering
void hideCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

// Reposition cursor to top-left instead of using system("cls")
void setCursorPosition(int x, int y) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hOut, coord);
}

// Flush keyboard buffer to clear stuck keypresses
void flushInputBuffer() {
    while (_kbhit()) {
        _getch();
    }
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
}

// Breadth-First Search (BFS) Pathfinding for Demon AI
Point getNextBfsStep(int startY, int startX, int targetY, int targetX) {
    if (startY == targetY && startX == targetX) return {startY, startX};

    bool visited[HEIGHT][WIDTH] = {false};
    Point parent[HEIGHT][WIDTH];
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            parent[i][j] = {-1, -1};
        }
    }

    std::queue<Point> q;
    q.push({startY, startX});
    visited[startY][startX] = true;

    int dy[] = {-1, 1, 0, 0};
    int dx[] = {0, 0, -1, 1};

    bool found = false;
    while (!q.empty()) {
        Point curr = q.front();
        q.pop();

        if (curr.y == targetY && curr.x == targetX) {
            found = true;
            break;
        }

        for (int i = 0; i < 4; i++) {
            int ny = curr.y + dy[i];
            int nx = curr.x + dx[i];

            if (ny >= 0 && ny < HEIGHT && nx >= 0 && nx < WIDTH) {
                if (!visited[ny][nx] && board[ny][nx] != WALL) {
                    visited[ny][nx] = true;
                    parent[ny][nx] = curr;
                    q.push({ny, nx});
                }
            }
        }
    }

    if (!found) return {startY, startX};

    // Reconstruct path to find immediate next step
    Point curr = {targetY, targetX};
    while (!(parent[curr.y][curr.x].y == startY && parent[curr.y][curr.x].x == startX)) {
        curr = parent[curr.y][curr.x];
        if (curr.y == -1 || curr.x == -1) break;
    }
    return curr;
}

void initialize(int currentLevel) {
    demons.clear();
    res = 0;
    invincible = false;
    invincibleEndTime = 0;

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (i == 0 || j == WIDTH - 1 || j == 0 || i == HEIGHT - 1) {
                board[i][j] = WALL;
            } else {
                board[i][j] = EMPTY;
            }
        }
    }

    // Procedural inner wall obstacles
    int wallCount = 35 + (currentLevel - 1) * 6;
    while (wallCount > 0) {
        int i = rand() % (HEIGHT - 2) + 1; 
        int j = rand() % (WIDTH - 2) + 1;
        if (board[i][j] == EMPTY) {
            board[i][j] = WALL;
            wallCount--;
        }
    }

    // Pacman spawn point at center
    pacman_x = WIDTH / 2;
    pacman_y = HEIGHT / 2;
    board[pacman_y][pacman_x] = PACMAN;

    // Spawn Demons (Level 1 = 4 demons, scaling to max 6)
    int demonCount = std::min(6, 4 + (currentLevel - 1));
    while (demonCount > 0) {
        int i = rand() % (HEIGHT - 2) + 1;
        int j = rand() % (WIDTH - 2) + 1;
        if (board[i][j] == EMPTY && (abs(i - pacman_y) + abs(j - pacman_x) > 6)) {
            demons.push_back({i, j, EMPTY});
            board[i][j] = DEMON;
            demonCount--;
        }
    }

    placePowerUps();
    placeFood();

    demonSpeed = std::max(150, 420 - (currentLevel - 1) * 40);
}

void drawBanner() {
    std::cout << "\033[1;33m";
    std::cout << "  ___   A R C A D E   P A C - M A N   ___\n";
    std::cout << " / _ \\                               / _ \\\n";
    std::cout << "| | | |  ==========================  | | | |\n";
    std::cout << " \\___/   D S A   C + +   E N G I N E  \\___/\033[0m\n";
    std::cout << "\033[1;36m========================================\033[0m\n";
}

void draw() {
    setCursorPosition(0, 0);

    drawBanner();

    std::string buffer = "";
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (board[i][j] == PACMAN) {
                buffer += "\033[1;33mC\033[0m"; // Yellow Pacman
            } else if (board[i][j] == DEMON) {
                if (invincible) {
                    buffer += "\033[1;34mX\033[0m"; // Blue Demon (Vulnerable)
                } else {
                    buffer += "\033[1;31mX\033[0m"; // Red Demon (Dangerous)
                }
            } else if (board[i][j] == FOOD) {
                buffer += "\033[1;32m.\033[0m"; // Green Food
            } else if (board[i][j] == POWERUP) {
                buffer += "\033[1;36mP\033[0m"; // Cyan Power-up
            } else if (board[i][j] == WALL) {
                buffer += "\033[1;37m#\033[0m"; // White Wall
            } else {
                buffer += " ";
            }
        }
        buffer += "\n";
    }

    std::cout << buffer;
    std::cout << "Score: " << score << " | High Score: " << std::max(score, highScore) 
              << " | Food Left: " << food << "    \n";
    std::cout << "Level: " << level << " | Invincible: ";
    if (invincible) {
        std::cout << "\033[1;36mYES (" << invincibleTimeLeft << "s)\033[0m   \n";
    } else {
        std::cout << "NO         \n";
    }
    if (paused) {
        std::cout << "\033[1;33m*** GAME PAUSED *** (Press 'P' to Resume)\033[0m\n";
    } else {
        std::cout << "Controls: ARROWS / WASD = Move | P = Pause | Q = Quit          \n";
    }
}

void movePacman(int move_x, int move_y) {
    int x = pacman_x + move_x;
    int y = pacman_y + move_y;

    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT || board[y][x] == WALL) {
        return;
    }

    if (board[y][x] == FOOD) {
        score += 10;
        food--;
        Beep(600, 20); 
        if (food <= 0) {
            res = 2; // Level cleared!
            return;
        }
    } else if (board[y][x] == DEMON) {
        if (!invincible) {
            res = 1; // Game Over
            return;
        } else {
            score += 50;
            Beep(1200, 60);
        }
    } else if (board[y][x] == POWERUP) {
        invincible = true;
        invincibleEndTime = GetTickCount() + INVINCIBILITY_DURATION_MS;
        score += 20;
        Beep(1000, 100); 
    }

    board[pacman_y][pacman_x] = EMPTY;
    pacman_x = x;
    pacman_y = y;
    board[pacman_y][pacman_x] = PACMAN;

    draw();
}

void moveDemons() {
    for (auto& demon : demons) {
        // Restore tile beneath demon before moving
        board[demon.y][demon.x] = demon.underTile;

        Point nextStep;
        int dist = abs(demon.x - pacman_x) + abs(demon.y - pacman_y);

        // BFS pathfinding if within range (or flee if invincible)
        if (dist <= 12 && !invincible) {
            nextStep = getNextBfsStep(demon.y, demon.x, pacman_y, pacman_x);
        } else {
            // Random exploratory step
            int dirs[4][2] = {{-1,0}, {1,0}, {0,-1}, {0,1}};
            int r = rand() % 4;
            int ny = demon.y + dirs[r][0];
            int nx = demon.x + dirs[r][1];

            if (ny > 0 && ny < HEIGHT - 1 && nx > 0 && nx < WIDTH - 1 && board[ny][nx] != WALL) {
                nextStep = {ny, nx};
            } else {
                nextStep = {demon.y, demon.x};
            }
        }

        // Check Pacman collision
        if (nextStep.y == pacman_y && nextStep.x == pacman_x) {
            if (!invincible) {
                res = 1;
                return;
            } else {
                score += 50;
                demon.y = 1;
                demon.x = 1;
                demon.underTile = EMPTY;
                board[1][1] = DEMON;
                continue;
            }
        }

        demon.y = nextStep.y;
        demon.x = nextStep.x;
        demon.underTile = (board[demon.y][demon.x] == DEMON) ? EMPTY : board[demon.y][demon.x];
        board[demon.y][demon.x] = DEMON;
    }
}

void placePowerUps() {
    int count = 4;
    while (count > 0) {
        int i = rand() % (HEIGHT - 2) + 1;
        int j = rand() % (WIDTH - 2) + 1;
        if (board[i][j] == EMPTY) {
            board[i][j] = POWERUP;
            count--;
        }
    }
}

// Evenly scatter 140 food pellets across the ENTIRE board
void placeFood() {
    food = 0;
    std::vector<Point> emptyCells;
    for (int i = 1; i < HEIGHT - 1; i++) {
        for (int j = 1; j < WIDTH - 1; j++) {
            if (board[i][j] == EMPTY) {
                emptyCells.push_back({i, j});
            }
        }
    }

    // Shuffle empty cells to scatter food randomly across the full maze
    std::random_shuffle(emptyCells.begin(), emptyCells.end());

    int targetFood = std::min((int)emptyCells.size(), TARGET_FOOD_COUNT);
    for (int k = 0; k < targetFood; k++) {
        int r = emptyCells[k].y;
        int c = emptyCells[k].x;
        board[r][c] = FOOD;
        food++;
    }
}

void showGameOver(bool win) {
    flushInputBuffer();
    system("cls");
    if (win) {
        std::cout << "\n\n  \033[1;32m========================================\033[0m\n";
        std::cout << "  \033[1;32m   LEVEL " << level << " CLEARED! BONUS +200 PTS   \033[0m\n";
        std::cout << "  \033[1;32m========================================\033[0m\n\n";
        Beep(800, 150); Beep(1000, 150); Beep(1200, 200);
        Sleep(1200);
    } else {
        if (score > highScore) highScore = score;
        std::cout << "\n\n  \033[1;31m========================================\033[0m\n";
        std::cout << "  \033[1;31m               GAME OVER!               \033[0m\n";
        std::cout << "  \033[1;31m========================================\033[0m\n";
        std::cout << "   Final Score: " << score << "\n";
        std::cout << "   High Score:  " << highScore << "\n\n";
        Beep(400, 200); Beep(300, 400);
        Sleep(1500);
    }
    flushInputBuffer();
}

void pauseGame() {
    paused = !paused;
    draw();
}

int main() {
    std::srand((unsigned int)std::time(0));
    hideCursor();
    system("cls");
    initialize(level);
    draw();

    DWORD lastGhostMove = GetTickCount();

    while (true) {
        DWORD now = GetTickCount();

        // Real-time invincibility countdown (ms based)
        if (invincible) {
            if (now >= invincibleEndTime) {
                invincible = false;
                invincibleTimeLeft = 0;
            } else {
                invincibleTimeLeft = (int)((invincibleEndTime - now) / 1000) + 1;
            }
        }

        if (res == 1) { // Player Died
            showGameOver(false);
            flushInputBuffer();
            std::cout << "Play Again? (Y/N): ";
            char choice;
            std::cin >> choice;
            if (choice == 'y' || choice == 'Y') {
                score = 0;
                level = 1;
                system("cls");
                initialize(level);
                draw();
                continue;
            } else {
                break;
            }
        }

        if (res == 2) { // Level Cleared!
            score += 200;
            showGameOver(true);
            level++;
            system("cls");
            initialize(level);
            draw();
            continue;
        }

        if (_kbhit()) { 
            char ch = _getch();
            if (ch == -32 || ch == 224 || ch == 0) { // Arrow Keys Prefix
                char arrow = _getch();
                switch (arrow) {
                    case 72: movePacman(0, -1); break; // UP
                    case 80: movePacman(0, 1);  break; // DOWN
                    case 75: movePacman(-1, 0); break; // LEFT
                    case 77: movePacman(1, 0);  break; // RIGHT
                }
            } else {
                switch (tolower(ch)) {
                    case 'w': movePacman(0, -1); break;
                    case 's': movePacman(0, 1); break;
                    case 'a': movePacman(-1, 0); break;
                    case 'd': movePacman(1, 0); break;
                    case 'p': pauseGame(); break;
                    case 'q': return 0;
                }
            }
        }

        if (!paused) {
            if (now - lastGhostMove >= (DWORD)demonSpeed) {
                moveDemons();
                draw();
                lastGhostMove = now;
            }
        }

        Sleep(25);
    }
    return 0;
}
