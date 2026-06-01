 #include <iostream>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include <windows.h>

using namespace std;

const int WIDTH  = 20;
const int HEIGHT = 20;

// Snake body
int tailX[400], tailY[400];
int tailLen = 0;

int snakeX, snakeY;
int fruitX, fruitY;
int score, highScore = 0;

enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };
Direction dir;

bool paused   = false;
bool gameOver = false;
int  speed    = 150; // milliseconds per tick

// ── Console helpers ──────────────────────────────────────────────────────────

void gotoXY(int x, int y) {
    COORD coord = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void hideCursor() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize   = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(h, &info);
}

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// ── Draw ─────────────────────────────────────────────────────────────────────

void draw() {
    gotoXY(0, 0);

    // Top border
    setColor(14); // yellow
    for (int i = 0; i < WIDTH + 2; i++) cout << "=";
    cout << "\n";

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH + 2; x++) {

            if (x == 0 || x == WIDTH + 1) {
                setColor(14);
                cout << "|";
            } else {
                int cx = x - 1;
                bool drawn = false;

                // Snake head
                if (cx == snakeX && y == snakeY) {
                    setColor(10); // bright green
                    cout << "O";
                    drawn = true;
                }

                // Snake tail
                if (!drawn) {
                    for (int k = 0; k < tailLen; k++) {
                        if (tailX[k] == cx && tailY[k] == y) {
                            setColor(2); // dark green
                            cout << "o";
                            drawn = true;
                            break;
                        }
                    }
                }

                // Fruit
                if (!drawn && cx == fruitX && y == fruitY) {
                    setColor(12); // red
                    cout << "*";
                    drawn = true;
                }

                if (!drawn) {
                    setColor(8); // dark grey
                    cout << ".";
                }
            }
        }
        cout << "\n";
    }

    // Bottom border
    setColor(14);
    for (int i = 0; i < WIDTH + 2; i++) cout << "=";
    cout << "\n";

    // HUD
    setColor(11);
    cout << " Score: " << score;
    cout << "   Best: " << highScore;
    cout << "   Length: " << tailLen + 1;
    cout << "   [P]ause  [R]estart  [ESC]Exit   \n";
    setColor(7);
}

// ── Input ─────────────────────────────────────────────────────────────────────

void input() {
    if (!_kbhit()) return;

    char key = _getch();

    // Arrow keys send two bytes: 0xE0 then the code
    if (key == (char)0xE0 || key == (char)0) {
        key = _getch();
        switch (key) {
            case 72: if (dir != DOWN)  dir = UP;    break; // Up arrow
            case 80: if (dir != UP)    dir = DOWN;  break; // Down arrow
            case 75: if (dir != RIGHT) dir = LEFT;  break; // Left arrow
            case 77: if (dir != LEFT)  dir = RIGHT; break; // Right arrow
        }
        return;
    }

    switch (key) {
        case 'w': case 'W': if (dir != DOWN)  dir = UP;    break;
        case 's': case 'S': if (dir != UP)    dir = DOWN;  break;
        case 'a': case 'A': if (dir != RIGHT) dir = LEFT;  break;
        case 'd': case 'D': if (dir != LEFT)  dir = RIGHT; break;
        case 'p': case 'P': paused = !paused; break;
        case 'r': case 'R': gameOver = true;  break; // triggers restart in main
        case 27:             exit(0);                // ESC = quit
    }
}

// ── Logic ─────────────────────────────────────────────────────────────────────

void spawnFruit() {
    bool onSnake;
    do {
        onSnake = false;
        fruitX = rand() % WIDTH;
        fruitY = rand() % HEIGHT;
        if (fruitX == snakeX && fruitY == snakeY) { onSnake = true; continue; }
        for (int k = 0; k < tailLen; k++) {
            if (tailX[k] == fruitX && tailY[k] == fruitY) { onSnake = true; break; }
        }
    } while (onSnake);
}

void logic() {
    if (paused || dir == STOP) return;

    // Shift tail
    int prevX = snakeX, prevY = snakeY;
    int tempX, tempY;
    for (int k = 0; k < tailLen; k++) {
        tempX    = tailX[k];
        tempY    = tailY[k];
        tailX[k] = prevX;
        tailY[k] = prevY;
        prevX    = tempX;
        prevY    = tempY;
    }

    // Move head
    switch (dir) {
        case UP:    snakeY--; break;
        case DOWN:  snakeY++; break;
        case LEFT:  snakeX--; break;
        case RIGHT: snakeX++; break;
        default: break;
    }

    // Wall collision
    if (snakeX < 0 || snakeX >= WIDTH || snakeY < 0 || snakeY >= HEIGHT) {
        gameOver = true;
        return;
    }

    // Self collision
    for (int k = 0; k < tailLen; k++) {
        if (tailX[k] == snakeX && tailY[k] == snakeY) {
            gameOver = true;
            return;
        }
    }

    // Eat fruit
    if (snakeX == fruitX && snakeY == fruitY) {
        score++;
        tailLen++;
        if (score > highScore) highScore = score;
        spawnFruit();
        // Speed up slightly every 5 fruits
        if (speed > 60 && score % 5 == 0) speed -= 10;
    }
}

// ── Init ──────────────────────────────────────────────────────────────────────

void initGame() {
    snakeX  = WIDTH  / 2;
    snakeY  = HEIGHT / 2;
    tailLen = 0;
    score   = 0;
    speed   = 150;
    dir     = STOP;
    paused  = false;
    gameOver= false;
    spawnFruit();
}

// ── Game over screen ──────────────────────────────────────────────────────────

void showGameOver() {
    gotoXY(0, HEIGHT + 3);
    setColor(12);
    cout << "\n  *** GAME OVER ***\n";
    setColor(11);
    cout << "  Score: " << score << "   Best: " << highScore << "\n";
    setColor(7);
    cout << "  [R] Play Again    [ESC] Quit\n\n";

    while (true) {
        if (_kbhit()) {
            char k = _getch();
            if (k == 'r' || k == 'R') return;   // restart
            if (k == 27)              exit(0);   // ESC
        }
        Sleep(50);
    }
}

// ── Main ──────────────────────────────────────────────────────────────────────

int main() {
    srand((unsigned)time(0));
    hideCursor();

    // Clear screen once
    system("cls");

    // Welcome
    setColor(10);
    cout << "\n\n  *** SNAKE ***\n\n";
    setColor(7);
    cout << "  Controls:\n";
    cout << "    WASD or Arrow Keys  - Move\n";
    cout << "    P                   - Pause / Resume\n";
    cout << "    R                   - Restart\n";
    cout << "    ESC                 - Quit\n\n";
    setColor(11);
    cout << "  Press any key to start...\n";
    setColor(7);
    _getch();
    system("cls");

    while (true) {
        initGame();

        while (!gameOver) {
            draw();
            input();
            logic();
            Sleep(speed);
        }

        draw();
        showGameOver();
        system("cls");
    }

    return 0;
}
