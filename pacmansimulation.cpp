#include <iostream>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include <windows.h>
#include <vector>
#include <queue>
#include <algorithm>
#include <string>

#define WIDTH 40
#define HEIGHT 20
#define PACMAN 'C'
#define WALL '#'
#define FOOD '.'
#define EMPTY ' '
#define DEMON 'X'
#define POWERUP 'P'
#define TARGET_FOOD_COUNT 140            // Rich food density across maze

enum Difficulty { EASY = 1, MEDIUM = 2, HARD = 3 };

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
int invincibilityDurationMs = 10000;
bool paused = false;
int level = 1; 
int demonSpeed = 350;
Difficulty selectedDifficulty = MEDIUM;
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
void selectDifficultyMenu();
std::string getCenterPadding(int textLength);

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

// Dynamically calculate left padding spaces to center text in any console window size
std::string getCenterPadding(int textLength) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int windowWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    if (windowWidth <= textLength) return "";
    return std::string((windowWidth - textLength) / 2, ' ');
}

// Flush keyboard buffer to clear stuck keypresses
void flushInputBuffer() {
    while (_kbhit()) {
        _getch();
    }
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
}

// Interactive Difficulty Selection Screen
void selectDifficultyMenu() {
    bool invalidChoice = false;
    while (true) {
        system("cls");
        std::string pad = getCenterPadding(50);
        std::cout << "\n\n";
        std::cout << pad << "\033[1;36m==================================================\033[0m\n";
        std::cout << pad << "\033[1;33m|    [C]     A R C A D E   P A C - M A N     [C] |\033[0m\n";
        std::cout << pad << "\033[1;33m|    ---    D S A   C + +   E N G I N E    ---   |\033[0m\n";
        std::cout << pad << "\033[1;36m==================================================\033[0m\n\n";

        std::cout << pad << "  \033[1;37mSELECT DIFFICULTY LEVEL:\033[0m\n\n";
        std::cout << pad << "  \033[1;32m[1] EASY   \033[0m - Slower Ghosts, 3 Ghosts, 15s Power-Up\n";
        std::cout << pad << "  \033[1;33m[2] MEDIUM \033[0m - Normal Ghosts, 4 Ghosts, 10s Power-Up\n";
        std::cout << pad << "  \033[1;31m[3] HARD   \033[0m - Fast Ghosts, 6 Ghosts, 7s Power-Up\n";
        std::cout << pad << "  \033[1;35m[Q] QUIT   \033[0m - Exit Game\n\n";

        if (invalidChoice) {
            std::cout << pad << "  \033[1;31m[!] Invalid Choice! Press 1, 2, 3 or Q to Quit.\033[0m\n\n";
        }

        std::cout << pad << "  Enter Choice (1-3 or Q): ";

        flushInputBuffer();
        char choice = (char)tolower(_getch());
        if (choice == '1') { selectedDifficulty = EASY; break; }
        else if (choice == '2') { selectedDifficulty = MEDIUM; break; }
        else if (choice == '3') { selectedDifficulty = HARD; break; }
        else if (choice == 'q') {
            system("cls");
            std::cout << "\n\n" << getCenterPadding(25) << "Thanks for playing Pac-Man!\n\n";
            exit(0);
        } else {
            invalidChoice = true;
        }
    }
    system("cls");
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
    invincibleTimeLeft = 0;

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

    // Base difficulty configuration
    int baseGhosts = (selectedDifficulty == EASY) ? 3 : (selectedDifficulty == HARD ? 5 : 4);
    int baseSpeed  = (selectedDifficulty == EASY) ? 500 : (selectedDifficulty == HARD ? 240 : 350);

    // Dynamic invincibility duration scaling per level
    if (selectedDifficulty == EASY) {
        if (currentLevel == 1) invincibilityDurationMs = 15000;
        else if (currentLevel == 2) invincibilityDurationMs = 10000;
        else invincibilityDurationMs = std::max(5000, 7000 - (currentLevel - 3) * 1000);
    } else if (selectedDifficulty == MEDIUM) {
        if (currentLevel == 1) invincibilityDurationMs = 10000;
        else if (currentLevel == 2) invincibilityDurationMs = 8000;
        else invincibilityDurationMs = std::max(4000, 6000 - (currentLevel - 3) * 1000);
    } else {
        if (currentLevel == 1) invincibilityDurationMs = 7000;
        else if (currentLevel == 2) invincibilityDurationMs = 5000;
        else invincibilityDurationMs = 4000;
    }

    // Spawn Demons
    int demonCount = std::min(7, baseGhosts + (currentLevel - 1));
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

    demonSpeed = std::max(120, baseSpeed - (currentLevel - 1) * 35);
}

void drawBanner() {
    std::string pad = getCenterPadding(50);
    std::cout << pad << "\033[1;36m==================================================\033[0m\n";
    std::cout << pad << "\033[1;33m|    [C]     A R C A D E   P A C - M A N     [C] |\033[0m\n";
    std::cout << pad << "\033[1;33m|    ---    D S A   C + +   E N G I N E    ---   |\033[0m\n";
    std::cout << pad << "\033[1;36m==================================================\033[0m\n";
}

void draw() {
    setCursorPosition(0, 0);

    drawBanner();

    std::string pad = getCenterPadding(WIDTH);

    for (int i = 0; i < HEIGHT; i++) {
        std::string line = pad;
        for (int j = 0; j < WIDTH; j++) {
            if (board[i][j] == PACMAN) {
                line += "\033[1;33mC\033[0m"; // Yellow Pacman
            } else if (board[i][j] == DEMON) {
                if (invincible) {
                    line += "\033[1;34mX\033[0m"; // Blue Demon (Vulnerable)
                } else {
                    line += "\033[1;31mX\033[0m"; // Red Demon (Dangerous)
                }
            } else if (board[i][j] == FOOD) {
                line += "\033[1;32m.\033[0m"; // Green Food
            } else if (board[i][j] == POWERUP) {
                line += "\033[1;36mP\033[0m"; // Cyan Power-up
            } else if (board[i][j] == WALL) {
                line += "\033[1;37m#\033[0m"; // White Wall
            } else {
                line += " ";
            }
        }
        std::cout << line << "\n";
    }

    std::string diffStr = (selectedDifficulty == EASY) ? "EASY" : (selectedDifficulty == HARD ? "HARD" : "MEDIUM");

    // Line 1: Score metrics dynamically centered
    std::string scoreStr = "Score: " + std::to_string(score) + " | High Score: " + std::to_string(std::max(score, highScore)) + " | Food Left: " + std::to_string(food);
    std::cout << getCenterPadding((int)scoreStr.length()) << scoreStr << "                          \n";

    // Line 2: Mode & Invincibility metrics dynamically centered
    std::string modePlain = "Mode: " + diffStr + " | Level: " + std::to_string(level) + " | Invincible: " + (invincible ? "YES (" + std::to_string(invincibleTimeLeft) + "s)" : "NO");
    std::cout << getCenterPadding((int)modePlain.length()) << "Mode: " << diffStr << " | Level: " << level << " | Invincible: ";
    if (invincible) {
        std::cout << "\033[1;36mYES (" << invincibleTimeLeft << "s)\033[0m                          \n";
    } else {
        std::cout << "NO                                      \n";
    }

    // Line 3: Pause / Controls line dynamically centered
    if (paused) {
        std::string pauseText = "*** GAME PAUSED *** (Press MOVE / SPACE / P to Resume | Q = Quit)";
        std::cout << getCenterPadding((int)pauseText.length()) << "\033[1;33m" << pauseText << "\033[0m                          \n";
    } else {
        std::string ctrlText = "Controls: ARROWS / WASD = Move | SPACE / P = Pause | Q = Quit";
        std::cout << getCenterPadding((int)ctrlText.length()) << ctrlText << "                          \n";
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
        invincibleEndTime = GetTickCount() + invincibilityDurationMs;
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
    std::string pad = getCenterPadding(44);

    if (win) {
        std::cout << "\n\n";
        std::cout << pad << "\033[1;32m============================================\033[0m\n";
        std::cout << pad << "\033[1;32m   LEVEL " << level << " CLEARED! BONUS +200 PTS   \033[0m\n";
        std::cout << pad << "\033[1;32m============================================\033[0m\n\n";
        Beep(800, 150); Beep(1000, 150); Beep(1200, 200);
        Sleep(1200);
    } else {
        if (score > highScore) highScore = score;
        std::cout << "\n\n";
        std::cout << pad << "\033[1;31m============================================\033[0m\n";
        std::cout << pad << "\033[1;31m               GAME OVER!                   \033[0m\n";
        std::cout << pad << "\033[1;31m============================================\033[0m\n";
        std::cout << pad << "   Final Score: " << score << "\n";
        std::cout << pad << "   High Score:  " << highScore << "\n\n";
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

    selectDifficultyMenu();

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
            std::string pad = getCenterPadding(30);
            std::cout << pad << "Play Again? (Y/N): ";
            char choice;
            std::cin >> choice;
            if (choice == 'y' || choice == 'Y') {
                score = 0;
                level = 1;
                selectDifficultyMenu();
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
            int moveX = 0, moveY = 0;
            bool isMoveKey = false;
            bool isPauseKey = false;
            bool isQuitKey = false;

            if (ch == -32 || ch == 224 || ch == 0) { // Arrow Keys Prefix
                char arrow = _getch();
                switch (arrow) {
                    case 72: moveY = -1; isMoveKey = true; break; // UP
                    case 80: moveY = 1;  isMoveKey = true; break; // DOWN
                    case 75: moveX = -1; isMoveKey = true; break; // LEFT
                    case 77: moveX = 1;  isMoveKey = true; break; // RIGHT
                }
            } else {
                switch (tolower(ch)) {
                    case 'w': moveY = -1; isMoveKey = true; break;
                    case 's': moveY = 1;  isMoveKey = true; break;
                    case 'a': moveX = -1; isMoveKey = true; break;
                    case 'd': moveX = 1;  isMoveKey = true; break;
                    case ' ':
                    case 'p': isPauseKey = true; break;
                    case 'q': isQuitKey = true; break;
                }
            }

            if (isQuitKey) {
                return 0;
            }

            if (paused) {
                if (isMoveKey) {
                    paused = false; // Unpause on movement!
                    movePacman(moveX, moveY);
                } else if (isPauseKey) {
                    paused = false; // Unpause on space/p!
                    draw();
                }
                // Any other key while paused is ignored!
            } else {
                if (isMoveKey) {
                    movePacman(moveX, moveY);
                } else if (isPauseKey) {
                    paused = true; // Pause game!
                    draw();
                }
                // Any other unassigned key is ignored!
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
