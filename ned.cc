#include <ncurses.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <vector>

std::ofstream logf;
#define LOGF logf
#define LOGFLUSH logf.flush()
bool quitNed = false;

struct CursorPosition {
  int x, y;
  CursorPosition() : x{0}, y{0} {}
  CursorPosition(int x, int y) : x{x}, y{y} {}
};

class Cursor {
 public:
  Cursor(WINDOW* window) : window{window}, position{} { moveFlush(); }
  Cursor(WINDOW* window, CursorPosition pos) : window{window}, position{pos} {
    moveFlush();
  }
  void move(int x, int y) {
    moveSet(x, y);
    moveFlush();
  }
  int getX() { return position.x; }
  int getY() { return position.y; }

 private:
  WINDOW
  *window;
  CursorPosition position;
  void moveSet(int x, int y) {
    position.x = x;
    position.y = y;
  }
  void moveFlush() { wmove(window, position.y, position.x); }
};

class Pane {
 public:
  Pane() {}
  Pane(WINDOW* window) : window{window} { cursors.push_back(Cursor(window)); }
  void walkCursors(int x, int y) {
        for (Cursor& c : cursors) {
      int newX = c.getX() + x;
      int newY = c.getY() + y;
      if (newX < 0) {
        newX = 0;
      } else if (newX > 50) {
        newX = 50;
      };
      if (newY < 0) {
        newY = 0;
      } else if (newY > 50) {
        newY = 50;
      };
      LOGF << "newX=" << newX << " newY=" << newY << std::endl;
      c.move(newX, newY);
    }
  }

 private:
  WINDOW* window;
  std::vector<Cursor> cursors;
};

Pane pane;

enum Key { KK_UP, KK_DOWN, KK_J, KK_K, KK_Q, KK_LEFT, KK_RIGHT, KK_COUNT };
Key getKeyPress() {
  int c = getch();
  if (c == -1)
    return KK_COUNT;
  switch (c) {
    case 'j':
      return KK_J;
    case 'k':
      return KK_K;
    case 'q':
      return KK_Q;
    case KEY_UP:
      return KK_UP;
    case KEY_DOWN:
      return KK_DOWN;
    case KEY_LEFT:
      return KK_LEFT;
    case KEY_RIGHT:
      return KK_RIGHT;
  }
  LOGF << "missing c: " << c << std::endl;
  return KK_COUNT;
}

void handleKeyPress(Key k) {
  switch (k) {
    case KK_UP:
      pane.walkCursors(0, -1);
      break;
    case KK_DOWN:
      pane.walkCursors(0, 1);
      break;
    case KK_LEFT:
      pane.walkCursors(-1, 0);
      break;
    case KK_RIGHT:
      pane.walkCursors(1, 0);
      break;
    default:
      break;
  }
}

void mainLoop() {
  Key k = getKeyPress();
  if (k != KK_COUNT) {
    LOGF << "key: " << k << "\r\n";
    handleKeyPress(k);
  }
  LOGFLUSH;
}

void signal_callback_handler(int signum) {
  endwin();
  exit(signum);
}

std::string toDirectory(std::string filepath) {
  size_t slashI = filepath.find_last_of("/");
  filepath = filepath.substr(0, slashI);
  return filepath;
}

int main(int argc, char** argv) {
  chdir(toDirectory(argv[0]).c_str());
  logf = std::ofstream("log.txt", std::ofstream::out | std::ofstream::trunc);
  if (!logf) {
    std::cerr << "Failed to open log.txt\n";
    return 1;
  }
  signal(SIGINT, signal_callback_handler);
  // setup ncurses
  WINDOW* w = initscr();
  halfdelay(1);
  keypad(w, true);
  noecho();
  nonl();
  intrflush(w, false);
  // destroy ncurses
  pane = Pane(w);
  while (!quitNed) {
    mainLoop();
  }
  endwin();
}
