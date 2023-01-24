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

// position of cursor on screen
struct CursorPosition {
  int x, y;
  CursorPosition() : x{0}, y{0} {}
  CursorPosition(int x, int y) : x{x}, y{y} {}
};

// data from file
struct EditBuffer {
  std::vector<std::string> lines;
};

// points to a specific character in a buffer
class BufferPosition {
 public:
  BufferPosition() : row{0}, col{0} {};
  BufferPosition(size_t r, size_t c) : row{r}, col{c} {};
  size_t row;
  size_t col;

 private:
};

class Cursor {
 public:
  Cursor(WINDOW* window) : window{window}, position{} { moveFlush(); }
  Cursor(WINDOW* window, CursorPosition pos) : window{window}, position{pos} {
    moveFlush();
  }
  void moveSet(int x, int y) {
    position.x = x;
    position.y = y;
  }
  void drawAt(int x, int y) {
    mvwchgat(window, y, x, 1, A_STANDOUT, 0, nullptr);
  }
  void moveFlush() {
    mvwchgat(window, position.y, position.x, 1, A_STANDOUT, 0, nullptr);
  }
  int getX() { return position.x; }
  int getY() { return position.y; }

 private:
  WINDOW
  *window;
  CursorPosition position;
};

/*scenarios
  right
    curor hits end of buffer
    cursor hits end of screen
  left
    cursor hits -1
    cursor hits beginning of screen

*/

class Pane {
 public:
  Pane() {}
  Pane(WINDOW* window) : window{window} {}
  Pane(WINDOW* window, EditBuffer& eb) : window{window}, buf{eb} {};
  void addCursor() { cursors.push_back(Cursor(window)); }
  void walkCursorsRight() {
    for (Cursor& c : cursors) {
      if (buf.lines.size() == 0) {
        c.moveSet(0, 0);
        continue;
      }
      int newX = c.getX() + 1;
      int newY = c.getY();
      if (newY < 0)
        newY = 0;
      if (newY >= (int)buf.lines.size())
        newY = buf.lines.size() - 1;
      if (c.getX() < 0) {
        newX = std::min(1, (int)buf.lines[newY].size());
        c.moveSet(newX, newY);
        continue;
      }
      bool isPastEOL = newX > (int)buf.lines[newY].size();
      if (!isPastEOL) {
        c.moveSet(newX, newY);
        continue;
      }
      // handle past EOL
      if (newY == (int)buf.lines.size() - 1) {
        // we are already at the bottom, just reset X to EOL;
        newX = (int)buf.lines[newY].size();
        c.moveSet(newX, newY);
        continue;
      }
      newY++;
      newX = 0;
      c.moveSet(newX, newY);
    }
  }
  void walkCursorsLeft() {
    for (Cursor& c : cursors) {
      if (buf.lines.size() == 0) {
        c.moveSet(0, 0);
        continue;
      }
      int newX = c.getX() - 1;
      int newY = c.getY();
      if (newY < 0)
        newY = 0;
      if (newY >= (int)buf.lines.size())
        newY = buf.lines.size() - 1;
      if (c.getX() > (int)buf.lines[newY].size()) {
        newX = std::max(0, (int)buf.lines[newY].size() - 1);
        c.moveSet(newX, newY);
        continue;
      }
      if (newX >= 0) {
        c.moveSet(newX, newY);
        continue;
      }
      if (newY == 0) {
        c.moveSet(0, 0);
        continue;
      }
      newY--;
      newX = buf.lines[newY].size();
      c.moveSet(newX, newY);
    }
  }
  void walkCursorsUp() {
    for (Cursor& c : cursors) {
      if (buf.lines.size() == 0) {
        c.moveSet(0, 0);
        continue;
      }
      int newX = c.getX();
      int newY = c.getY() - 1;
      if (newY < 0) {
        c.moveSet(0, 0);
        continue;
      }
      if (newY >= (int)buf.lines.size() - 1)
        newY = std::max((int)buf.lines.size() - 2, 0);
      c.moveSet(newX, newY);
    }
  }
  void walkCursorsDown() {
    for (Cursor& c : cursors) {
      if (buf.lines.size() == 0) {
        c.moveSet(0, 0);
        continue;
      }
      int newX = c.getX();
      int newY = c.getY() + 1;
      if (newY < 0) {
        newY = std::min(1, (int)buf.lines.size());
        c.moveSet(newX, newY);
        continue;
      }
      if (newY > (int)buf.lines.size() - 1)
        newY = buf.lines.size() - 1;
      c.moveSet(newX, newY);
    }
  }
  void drawCursors() {
    for (Cursor& c : cursors) {
      if (buf.lines.size() == 0) {
        c.drawAt(0, 0);
        continue;
      }
      int x = c.getX();
      int y = c.getY();
      if (y < 0)
        y = 0;
      if (y > (int)buf.lines.size() - 1)
        y = (int)buf.lines.size() - 1;
      if (x < 0)
        x = 0;
      if (x > (int)buf.lines[y].size())
        x = buf.lines[y].size();
      c.drawAt(x, y);
    }
  }

  void drawBuffer() {
    int maxX, maxY;
    getmaxyx(window, maxY, maxX);
    size_t row = bufOffset.row;
    size_t bottomEndY = std::min(row + maxY, buf.lines.size());
    for (; row < bottomEndY; row++) {
      size_t col = bufOffset.col;
      size_t rightEndX = std::min(col + maxX, buf.lines[row].size());
      wmove(window, row, col);
      for (; col < rightEndX; col++) {
        waddch(window, buf.lines[row].at(col));
      }
    }
  }

 private:
  WINDOW* window;
  EditBuffer buf;
  BufferPosition
      bufOffset;  // the postion in buf that aligns with screen position 0, 0
  std::vector<Cursor> cursors;
};

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

void walkCursors(Key k, Pane& pane) {
  switch (k) {
    case KK_UP:
      pane.walkCursorsUp();
      break;
    case KK_DOWN:
      pane.walkCursorsDown();
      break;
    case KK_LEFT:
      pane.walkCursorsLeft();
      break;
    case KK_RIGHT:
      pane.walkCursorsRight();
      break;
    default:
      break;
  }
}

void mainLoop(Pane& pane) {
  Key k = getKeyPress();
  if (k != KK_COUNT) {
    if (k == KK_UP || k == KK_DOWN || k == KK_LEFT || k == KK_RIGHT) {
      erase();
      pane.drawBuffer();
      walkCursors(k, pane);
      pane.drawCursors();
    }
    LOGF << "key: " << k << "\r\n";
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
  std::vector<std::string> lines(
      {"this is a rearendering code.", "is", "a", "test", "yo!", ""});
  EditBuffer buf{lines};
  // setup ncurses
  WINDOW* w = initscr();
  halfdelay(1);
  keypad(w, true);
  noecho();
  nonl();
  curs_set(0);  // hide the builtin cursor
  intrflush(w, false);
  // destroy ncurses
  Pane pane{w, buf};
  pane.drawBuffer();
  pane.addCursor();
  while (!quitNed) {
    mainLoop(pane);
  }
  endwin();
}
