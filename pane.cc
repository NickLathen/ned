#include "pane.hh"
#include <ncurses.h>

BufferCursor::BufferCursor(WINDOW* window, EditBuffer buf)
    : window{window}, position{}, buf{buf} {};
BufferCursor::BufferCursor(WINDOW* window, EditBuffer buf, BufferPosition pos)
    : window{window}, position{pos}, buf{buf} {};
void BufferCursor::moveSet(int x, int y) {
  position.row = y;
  position.col = x;
}
int BufferCursor::getX() {
  return position.col;
}
int BufferCursor::getY() {
  return position.row;
}
void BufferCursor::walkRight() {
  if (buf.lines.size() == 0) {
    moveSet(0, 0);
    return;
  }
  int newX = getX() + 1;
  int newY = getY();
  if (newY < 0)
    newY = 0;
  if (newY >= (int)buf.lines.size())
    newY = buf.lines.size() - 1;
  if (getX() < 0) {
    newX = std::min(1, (int)buf.lines[newY].size());
    moveSet(newX, newY);
    return;
  }
  bool isPastEOL = newX > (int)buf.lines[newY].size();
  if (!isPastEOL) {
    moveSet(newX, newY);
    return;
  }
  // handle past EOL
  if (newY == (int)buf.lines.size() - 1) {
    // we are already at the bottom, just reset X to EOL;
    newX = (int)buf.lines[newY].size();
    moveSet(newX, newY);
    return;
  }
  newY++;
  newX = 0;
  moveSet(newX, newY);
};
void BufferCursor::walkLeft() {
  if (buf.lines.size() == 0) {
    moveSet(0, 0);
    return;
  }
  int newX = getX() - 1;
  int newY = getY();
  if (newY < 0)
    newY = 0;
  if (newY >= (int)buf.lines.size())
    newY = buf.lines.size() - 1;
  if (getX() > (int)buf.lines[newY].size()) {
    newX = std::max(0, (int)buf.lines[newY].size() - 1);
    moveSet(newX, newY);
    return;
  }
  if (newX >= 0) {
    moveSet(newX, newY);
    return;
  }
  if (newY == 0) {
    moveSet(0, 0);
    return;
  }
  newY--;
  newX = buf.lines[newY].size();
  moveSet(newX, newY);
};
void BufferCursor::walkUp() {
  if (buf.lines.size() == 0) {
    moveSet(0, 0);
    return;
  }
  int newX = getX();
  int newY = getY() - 1;
  if (newY < 0) {
    moveSet(0, 0);
    return;
  }
  if (newY >= (int)buf.lines.size() - 1)
    newY = std::max((int)buf.lines.size() - 2, 0);
  moveSet(newX, newY);
};
void BufferCursor::walkDown() {
  if (buf.lines.size() == 0) {
    moveSet(0, 0);
    return;
  }
  int newX = getX();
  int newY = getY() + 1;
  if (newY < 0) {
    newY = std::min(1, (int)buf.lines.size());
    moveSet(newX, newY);
    return;
  }
  if (newY > (int)buf.lines.size() - 1)
    newY = buf.lines.size() - 1;
  moveSet(newX, newY);
};
void BufferCursor::drawOffset(const BufferPosition& bufOffset) {
  if (buf.lines.size() == 0) {
    mvwchgat(window, 0, 0, 1, A_STANDOUT, 0, nullptr);
    return;
  }
  int x = getX();
  int y = getY();
  if (y < 0)
    y = 0;
  if (y > (int)buf.lines.size() - 1)
    y = (int)buf.lines.size() - 1;
  if (x < 0)
    x = 0;
  if (x > (int)buf.lines[y].size())
    x = buf.lines[y].size();
  mvwchgat(window, y, x, 1, A_STANDOUT, 0, nullptr);
}

Pane::Pane(){};
Pane::Pane(WINDOW* window) : window{window} {};
Pane::Pane(WINDOW* window, EditBuffer& eb) : window{window}, buf{eb} {};
void Pane::addCursor() {
  cursors.push_back(BufferCursor(window, buf));
}
void Pane::walkCursorsRight() {
  for (BufferCursor& c : cursors) {
    c.walkRight();
  }
}
void Pane::walkCursorsLeft() {
  for (BufferCursor& c : cursors) {
    c.walkLeft();
  }
}
void Pane::walkCursorsUp() {
  for (BufferCursor& c : cursors) {
    c.walkUp();
  }
}
void Pane::walkCursorsDown() {
  for (BufferCursor& c : cursors) {
    c.walkDown();
  }
}
void Pane::drawCursors() {
  for (BufferCursor& c : cursors) {
    c.drawOffset(bufOffset);
  }
}

void Pane::drawBuffer() {
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
