#include <iostream>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include <windows.h>

#define WIDTH 40
#define HEIGHT 20
#define PACMAN 'C'
#define WALL '#'
#define FOOD '.'
#define EMPTY ' '
#define DEMON 'X'
#define POWERUP 'P'
#define INVINCIBILITY_DURATION 10
#define MIN_FOOD 50 
#define DEMON_SPEED_INCREASE 40

int res = 0;
int score = 0;
int pacman_x, pacman_y;
char board[HEIGHT][WIDTH];
int food = 0;
int curr = 0;
bool invincible = false;
int invincibleTime = 0; 
bool paused = false;
int level = 1; 
int demonSpeed = 500;


void initialize(int level = 1);
void draw();
void move(int move_x, int move_y);
void moveDemons();
void placePowerUps();
void placeFood();
void showGameOver(bool win);
void pauseGame();

void initialize(int level) {

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (i == 0 || j == WIDTH - 1 || j == 0 || i == HEIGHT - 1) {
                board[i][j] = WALL;
            } else {
                board[i][j] = EMPTY;
            }
        }
    }

    int wallCount = 50 + (level - 1) * 10;
    while (wallCount > 0) {
        int i = rand() % (HEIGHT - 2) + 1; 
        int j = rand() % (WIDTH - 2) + 1;
        if (board[i][j] == EMPTY) {
            board[i][j] = WALL;
            wallCount--;
        }
    }

    int demonCount = 5 + (level - 1) * 2;
    while (demonCount > 0) {
        int i = rand() % (HEIGHT - 2) + 1;
        int j = rand() % (WIDTH - 2) + 1;
        if (board[i][j] == EMPTY) {
            board[i][j] = DEMON;
            demonCount--;
        }
    }

    placePowerUps();

    placeFood();

    pacman_x = WIDTH / 2;
    pacman_y = HEIGHT / 2;
    board[pacman_y][pacman_x] = PACMAN;
}


void draw() {
    system("cls");
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (board[i][j] == PACMAN) {
                std::cout << "\033[1;33m" << PACMAN << "\033[0m"; // Yellow
            } else if (board[i][j] == DEMON) {
                std::cout << "\033[1;31m" << DEMON << "\033[0m"; // Red
            } else if (board[i][j] == FOOD) {
                std::cout << "\033[1;32m" << FOOD << "\033[0m"; // Green
            } else if (board[i][j] == POWERUP) {
                std::cout << "\033[1;36m" << POWERUP << "\033[0m"; // Cyan
            } else {
                std::cout << board[i][j];
            }
        }
        std::cout << "\n";
    }
    std::cout << "Score: " << score << " | Food Left: " << food << " | Invincible: " << (invincible ? "Yes" : "No") << "\n";
    std::cout << "Level: " << level << " | Demon Speed: " << (1000 - demonSpeed) << "\n";
    if (paused) {
        std::cout << "Game Paused. Press 'p' to resume.\n";
    }
}

void move(int move_x, int move_y) {
    int x = pacman_x + move_x;
    int y = pacman_y + move_y;

    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT || board[y][x] == WALL) {
        return;
    }

    if (board[y][x] == FOOD) {
        score += 10;
        food--;
        curr++;
        Beep(500, 200); 
        if (food == 0) {
            res = 2; 
            return;
        }
    } else if (board[y][x] == DEMON) {
        if (!invincible) {
            res = 1;
            return;
        }
    } else if (board[y][x] == POWERUP) {
        invincible = true;
        invincibleTime = INVINCIBILITY_DURATION;
        Beep(1000, 500); 
    }

    board[pacman_y][pacman_x] = EMPTY;
    pacman_x = x;
    pacman_y = y;
    board[pacman_y][pacman_x] = PACMAN;

    draw();
}


void moveDemons() {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (board[i][j] == DEMON) {
                int dx = pacman_x - j;
                int dy = pacman_y - i;
                int distance = abs(dx) + abs(dy);

                if (distance <= 5) {
                    if (abs(dx) > abs(dy)) {
                        if (dx > 0 && board[i][j + 1] != WALL && board[i][j + 1] != DEMON) {
                            if (board[i][j + 1] == PACMAN && !invincible) {
                                res = 1;
                                return;
                            }
                            board[i][j] = EMPTY;
                            board[i][j + 1] = DEMON;
                        } else if (dx < 0 && board[i][j - 1] != WALL && board[i][j - 1] != DEMON) {
                            if (board[i][j - 1] == PACMAN && !invincible) {
                                res = 1;
                                return;
                            }
                            board[i][j] = EMPTY;
                            board[i][j - 1] = DEMON;
                        }
                    } else {
                        if (dy > 0 && board[i + 1][j] != WALL && board[i + 1][j] != DEMON) {
                            if (board[i + 1][j] == PACMAN && !invincible) {
                                res = 1;
                                return;
                            }
                            board[i][j] = EMPTY;
                            board[i + 1][j] = DEMON;
                        } else if (dy < 0 && board[i - 1][j] != WALL && board[i - 1][j] != DEMON) {
                            if (board[i - 1][j] == PACMAN && !invincible) {
                                res = 1;
                                return;
                            }
                            board[i][j] = EMPTY;
                            board[i - 1][j] = DEMON;
                        }
                    }
                } else {
                    int direction = rand() % 4;
                    int new_i = i, new_j = j;
                    switch (direction) {
                        case 0: new_i--; break;
                        case 1: new_i++; break;
                        case 2: new_j--; break;
                        case 3: new_j++; break;
                    }
                    if (new_i >= 0 && new_i < HEIGHT && new_j >= 0 && new_j < WIDTH &&
                        board[new_i][new_j] != WALL && board[new_i][new_j] != DEMON) {
                        if (board[new_i][new_j] == PACMAN && !invincible) {
                            res = 1;
                            return;
                        }
                        board[i][j] = EMPTY;
                        board[new_i][new_j] = DEMON;
                    }
                }
            }
        }
    }
}

void placePowerUps() {
    int count = 3;
    while (count > 0) {
        int i = rand() % (HEIGHT - 2) + 1;
        int j = rand() % (WIDTH - 2) + 1;
        if (board[i][j] == EMPTY) {
            board[i][j] = POWERUP;
            count--;
        }
    }
}

void placeFood() {
    food = 0;
    while (food < MIN_FOOD) {
        for (int i = 1; i < HEIGHT - 1; i++) {
            for (int j = 1; j < WIDTH - 1; j++) {
                if (board[i][j] == EMPTY && rand() % 5 == 0) {
                    board[i][j] = FOOD;
                    food++;
                    if (food >= MIN_FOOD) break;
                }
            }
            if (food >= MIN_FOOD) break;
        }
    }
}

void showGameOver(bool win) {
    system("cls");
    if (win) {
        std::cout << "\033[1;32mYOU WIN!\033[0m\n";
    } else {
        std::cout << "\033[1;31mGAME OVER!\033[0m\n";
    }
    std::cout << "Your Score: " << score << "\n";
    Sleep(2000); 
}

void pauseGame() {
    paused = !paused;
    if (paused) {
        std::cout << "Game Paused. Press 'p' to resume.\n";
    } else {
        draw();
    }
}

int main() {
    std::srand(std::time(0));
    initialize();
    draw(); 
    while (true) {
        if (res == 1) {
            showGameOver(false);
            break;
        }
        if (res == 2) {
            showGameOver(true);
            break;
        }

        if (invincible) {
            invincibleTime--;
            if (invincibleTime <= 0) {
                invincible = false;
            }
        }

        if (_kbhit()) { 
            char ch = _getch();
            switch (ch) {
                case 'w': move(0, -1); break;
                case 's': move(0, 1); break;
                case 'a': move(-1, 0); break;
                case 'd': move(1, 0); break;
                case 'p': pauseGame(); break;
                case 'q': return 0;
            }
        }

        if (!paused) {
            moveDemons();
            Sleep(demonSpeed);
        }
    }
    return 0;
}
