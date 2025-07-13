#include <iostream>
#include <conio.h>
#include <windows.h>
#include <fstream>
#include <ctime>
using namespace std;

#define MAX_LENGTH 1000

enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };
enum Color { BLACK = 0, GREEN = 2, RED = 4, YELLOW = 6, WHITE = 15 };

int consoleWidth = 60, consoleHeight = 25;
bool paused = false;

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void gotoxy(int x, int y) {
    COORD coord{ (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void hideCursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

struct Point {
    int x, y;
    Point() {}
    Point(int _x, int _y) : x(_x), y(_y) {}
    bool operator==(const Point& p) const {
        return x == p.x && y == p.y;
    }
};

class Snake {
public:
    Point body[MAX_LENGTH];
    int length;
    Direction dir;

    Snake(int x, int y) {
        length = 1;
        body[0] = Point(x, y);
        dir = RIGHT;
    }

    void changeDirection(Direction newDir) {
        if ((dir == UP && newDir == DOWN) || (dir == DOWN && newDir == UP) ||
            (dir == LEFT && newDir == RIGHT) || (dir == RIGHT && newDir == LEFT))
            return;
        dir = newDir;
    }

    bool move(Point food) {
        Point tail = body[length - 1];
        for (int i = length - 1; i > 0; i--)
            body[i] = body[i - 1];

        switch (dir) {
        case UP: body[0].y--; break;
        case DOWN: body[0].y++; break;
        case LEFT: body[0].x--; break;
        case RIGHT: body[0].x++; break;
        }

        if (body[0].x <= 0 || body[0].x >= consoleWidth - 1 ||
            body[0].y <= 1 || body[0].y >= consoleHeight - 1)
            return false;

        for (int i = 1; i < length; i++)
            if (body[0] == body[i])
                return false;

        if (body[0] == food) {
            body[length++] = tail;
            Beep(700, 80);
            return true;
        }

        return true;
    }
};

class Game {
    Snake* snake;
    Point food;
    int score = 0;
    int level = 1;
    int delay = 150;

public:
    Game() {
        hideCursor();
        srand(time(0));
        snake = new Snake(consoleWidth / 2, consoleHeight / 2);
        spawnFood();
    }

    ~Game() {
        delete snake;
    }

    void drawBorder() {
        setColor(WHITE);
        for (int x = 0; x < consoleWidth; x++) {
            gotoxy(x, 1); cout << "=";
            gotoxy(x, consoleHeight - 1); cout << "=";
        }
        for (int y = 2; y < consoleHeight - 1; y++) {
            gotoxy(0, y); cout << "|";
            gotoxy(consoleWidth - 1, y); cout << "|";
        }
    }

    void spawnFood() {
        while (true) {
            int x = 1 + rand() % (consoleWidth - 3);
            int y = 2 + rand() % (consoleHeight - 4);
            food = Point(x, y);
            bool onSnake = false;
            for (int i = 0; i < snake->length; i++)
                if (snake->body[i] == food) {
                    onSnake = true;
                    break;
                }
            if (!onSnake) break;
        }
    }

    void drawUI() {
        setColor(YELLOW);
        gotoxy(2, 0);
        cout << "Score: " << score << "  Length: " << snake->length << "  Level: " << level;
        if (paused) {
            gotoxy(consoleWidth / 2 - 5, 0);
            cout << "[PAUSED]";
        }
    }

    void draw() {
        drawBorder();
        drawUI();

        // Draw Snake
        for (int i = 0; i < snake->length; i++) {
            gotoxy(snake->body[i].x, snake->body[i].y);
            setColor(i == 0 ? RED : GREEN);
            cout << (i == 0 ? '@' : 'O');
        }

        // Draw food
        gotoxy(food.x, food.y);
        setColor(WHITE);
        cout << '*';

        setColor(WHITE);
    }

    void clear() {
        for (int i = 0; i < snake->length; i++) {
            gotoxy(snake->body[i].x, snake->body[i].y);
            cout << ' ';
        }
        gotoxy(food.x, food.y);
        cout << ' ';
    }

    void getInput() {
        if (_kbhit()) {
            char key = _getch();
            switch (key) {
            case 'w': case 'W': snake->changeDirection(UP); break;
            case 'a': case 'A': snake->changeDirection(LEFT); break;
            case 's': case 'S': snake->changeDirection(DOWN); break;
            case 'd': case 'D': snake->changeDirection(RIGHT); break;
            case 'p': case 'P': paused = !paused; break;
            case 'q': case 'Q': exit(0);
            }
        }
    }

    void update() {
        if (paused) return;
        bool alive = snake->move(food);
        if (!alive) {
            gameOver();
            return;
        }
        if (snake->body[0] == food) {
            score += 10;
            if (score % 50 == 0 && delay > 60) {
                level++;
                delay -= 10;
            }
            spawnFood();
        }
    }

    void gameOver() {
        system("cls");
        int high = loadHighScore();
        if (score > high) saveHighScore(score);
        gotoxy(consoleWidth / 2 - 6, consoleHeight / 2 - 1);
        cout << "GAME OVER!";
        gotoxy(consoleWidth / 2 - 8, consoleHeight / 2);
        cout << "Final Score: " << score;
        gotoxy(consoleWidth / 2 - 8, consoleHeight / 2 + 1);
        cout << "High Score: " << max(score, high);
        gotoxy(consoleWidth / 2 - 10, consoleHeight / 2 + 3);
        cout << "Press R to Restart or Q to Quit";

        while (true) {
            if (_kbhit()) {
                char key = _getch();
                if (key == 'r' || key == 'R') {
                    delete snake;
                    snake = new Snake(consoleWidth / 2, consoleHeight / 2);
                    score = 0; level = 1; delay = 150;
                    spawnFood();
                    system("cls");
                    break;
                } else if (key == 'q' || key == 'Q') exit(0);
            }
        }
    }

    void run() {
        while (true) {
            getInput();
            clear();
            update();
            draw();
            Sleep(delay);
        }
    }

    int loadHighScore() {
        ifstream in("highscore.txt");
        int hs = 0;
        if (in) in >> hs;
        return hs;
    }

    void saveHighScore(int s) {
        ofstream out("highscore.txt");
        out << s;
    }
};

void showIntro() {
    system("cls");
    setColor(YELLOW);
    gotoxy(consoleWidth / 2 - 6, consoleHeight / 2 - 3);
    cout << "SNAKE GAME";
    gotoxy(consoleWidth / 2 - 15, consoleHeight / 2 - 1);
    cout << "Use W A S D to move | P = Pause | Q = Quit";
    gotoxy(consoleWidth / 2 - 10, consoleHeight / 2 + 1);
    cout << "Press any key to start...";
    _getch();
    system("cls");
}

int main() {
    showIntro();
    Game game;
    game.run();
    return 0;
}
