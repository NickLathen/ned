#include "pane.hh"
#include <ncurses.h>
#include <iostream>
#include "log.h"

BufferCursor::BufferCursor(WINDOW* window, EditBuffer buf)
    : window{window}, position{}, buf{buf} {};
BufferCursor::BufferCursor(WINDOW* window, EditBuffer buf, BufferPosition pos)
    : window{window}, position{pos}, buf{buf} {};
void BufferCursor::moveSet(int x, int y) {
  LOGF << "moveset: " << x << ", " << y << std::endl;
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
void BufferCursor::drawOffset(BufferPosition& bufOffset) {
  int maxX, maxY;
  getmaxyx(window, maxY, maxX);
  int bufX = getX();
  int bufY = getY();
  if (bufX > (int)buf.lines[bufY].size()) {
    bufX = buf.lines[bufY].size();
  }
  int screenX = bufX - bufOffset.col;
  int screenY = bufY - bufOffset.row;
  // ignore offscreen
  if (screenX < 0 || screenX >= maxX || screenY < 0 || screenY >= maxY)
    return;

  mvwchgat(window, screenY, screenX, 1, A_STANDOUT, 0, nullptr);
}
void BufferCursor::adjustOffset(BufferPosition& bufOffset) {
  int maxX, maxY;
  getmaxyx(window, maxY, maxX);
  int bufX = getX();
  int bufY = getY();
  if (bufX > (int)buf.lines[bufY].size()) {
    bufX = buf.lines[bufY].size();
  }
  int screenX = bufX - bufOffset.col;
  int screenY = bufY - bufOffset.row;
  if (screenX < 4) {
    bufOffset.col = std::max(bufX - 4, 0);
  } else if (screenX >= maxX - 4) {
    bufOffset.col = bufX - maxX + 5;
  }
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
  if (buf.lines.size() == 0)
    return;
  int maxX, maxY;
  getmaxyx(window, maxY, maxX);
  int rowOffset = bufOffset.row;
  int colOffset = bufOffset.col;
  for (int row = 0; row < maxY; row++) {
    if (row + rowOffset >= (int)buf.lines.size()) {
      break;
    }
    wmove(window, row, 0);
    for (int col = 0; col < maxX; col++) {
      if (col + colOffset >= (int)buf.lines[row].size())
        break;
      waddch(window, buf.lines[row + rowOffset].at(col + colOffset));
    }
  }
}

void Pane::adjustOffset() {
  if (cursors.size() == 0)
    return;
  cursors[0].adjustOffset(bufOffset);
}

void Pane::setOffset(size_t row, size_t col) {
  bufOffset.row = row;
  bufOffset.col = col;
}
