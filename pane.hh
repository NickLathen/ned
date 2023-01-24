#pragma once
#include <ncurses.h>
#include <string>
#include <vector>

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

class BufferCursor {
 public:
  BufferCursor(WINDOW* window, EditBuffer buf);
  BufferCursor(WINDOW* window, EditBuffer buf, BufferPosition pos);
  void walkRight();
  void walkLeft();
  void walkUp();
  void walkDown();
  void moveSet(int x, int y);
  void drawOffset(const BufferPosition& bufOffset);
  int getX();
  int getY();

 private:
  WINDOW
  *window;
  BufferPosition position;
  EditBuffer buf;
};

class Pane {
 public:
  Pane();
  Pane(WINDOW* window);
  Pane(WINDOW* window, EditBuffer& eb);
  void addCursor();
  void walkCursorsRight();
  void walkCursorsLeft();
  void walkCursorsUp();
  void walkCursorsDown();
  void drawCursors();
  void drawBuffer();

 private:
  WINDOW* window;
  EditBuffer buf;
  BufferPosition bufOffset;
  std::vector<BufferCursor> cursors;
};
