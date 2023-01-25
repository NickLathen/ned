#pragma once
#include <ncurses.h>
#include <string>
#include <vector>

// data from file
struct EditBuffer {
  std::vector<std::string> lines;
};

// points to a specific character in a buffer
struct BufferPosition {
  size_t row, col;
};

class BufferCursor {
 public:
  BufferCursor(WINDOW* window, EditBuffer buf);
  BufferCursor(WINDOW* window, EditBuffer buf, BufferPosition pos);
  int getX();
  int getY();
  void walkRight();
  void walkLeft();
  void walkUp();
  void walkDown();
  void moveSet(int x, int y);
  void drawOffset(BufferPosition& bufOffset);
  void adjustOffset(BufferPosition& bufOffset);

 private:
  WINDOW* window;
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
  void adjustOffset();
  void setOffset(size_t row, size_t col);

 private:
  WINDOW* window;
  EditBuffer buf;
  BufferPosition bufOffset;
  std::vector<BufferCursor> cursors;
};
