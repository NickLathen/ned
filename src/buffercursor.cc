#include <iostream>
#include "pane.hh"

BufferCursor::BufferCursor() {}
BufferCursor::BufferCursor(BufferPosition& pos) : position{pos} {}
void BufferCursor::moveSet(int x, int y) {
  std::cout << "moveset: " << x << ", " << y << std::endl;
  position.row = y;
  position.col = x;
  tailPosition = position;
}
void BufferCursor::selectSet(int x, int y) {
  std::cout << "selectSet: " << x << ", " << y << std::endl;
  position.row = y;
  position.col = x;
}
void BufferCursor::selectUp(const EditBuffer& buf) {
  if (buf.lines.size() == 0) {
    selectSet(0, 0);
    return;
  }
  int newX = position.col;
  int newY = position.row - 1;
  if (newY < 0) {
    selectSet(0, 0);
    return;
  }
  if (newY >= (int)buf.lines.size() - 1)
    newY = std::max((int)buf.lines.size() - 2, 0);
  selectSet(newX, newY);
}
void BufferCursor::selectDown(const EditBuffer& buf) {
  if (buf.lines.size() == 0) {
    selectSet(0, 0);
    return;
  }
  int newX = position.col;
  int newY = position.row + 1;
  if (newY < 0) {
    newY = std::min(1, (int)buf.lines.size());
    selectSet(newX, newY);
    return;
  }
  if (newY > (int)buf.lines.size() - 1)
    newY = buf.lines.size() - 1;
  selectSet(newX, newY);
}
void BufferCursor::selectLeft(const EditBuffer& buf) {
  if (buf.lines.size() == 0) {
    selectSet(0, 0);
    return;
  }
  int newX = position.col - 1;
  int newY = position.row;
  if (newY < 0)
    newY = 0;
  if (newY >= (int)buf.lines.size())
    newY = buf.lines.size() - 1;
  if (position.col > buf.lines[newY].size()) {
    newX = std::max(0, (int)buf.lines[newY].size() - 1);
    selectSet(newX, newY);
    return;
  }
  if (newX >= 0) {
    selectSet(newX, newY);
    return;
  }
  if (newY == 0) {
    selectSet(0, 0);
    return;
  }
  newY--;
  newX = buf.lines[newY].size();
  selectSet(newX, newY);
}
void BufferCursor::selectRight(const EditBuffer& buf) {
  if (buf.lines.size() == 0) {
    selectSet(0, 0);
    return;
  }
  int newX = position.col + 1;
  int newY = position.row;
  if (newY < 0)
    newY = 0;
  if (newY >= (int)buf.lines.size())
    newY = buf.lines.size() - 1;
  bool isPastEOL = newX > (int)buf.lines[newY].size();
  if (!isPastEOL) {
    selectSet(newX, newY);
    return;
  }
  // handle past EOL
  if (newY == (int)buf.lines.size() - 1) {
    // we are already at the bottom, just reset X to EOL;
    newX = (int)buf.lines[newY].size();
    selectSet(newX, newY);
    return;
  }
  newY++;
  newX = 0;
  selectSet(newX, newY);
}
void BufferCursor::moveUp(const EditBuffer& buf) {
  selectUp(buf);
  tailPosition = position;
}
void BufferCursor::moveDown(const EditBuffer& buf) {
  selectDown(buf);
  tailPosition = position;
}
void BufferCursor::moveLeft(const EditBuffer& buf) {
  selectLeft(buf);
  tailPosition = position;
}
void BufferCursor::moveRight(const EditBuffer& buf) {
  selectRight(buf);
  tailPosition = position;
}
BufferPosition BufferCursor::getPosition() const {
  return position;
}
BufferPosition BufferCursor::getTailPosition() const {
  return tailPosition;
}
size_t BufferCursor::getRow() const {
  return position.row;
}
size_t BufferCursor::getCol() const {
  return position.col;
}
size_t BufferCursor::getTailRow() const {
  return tailPosition.row;
}
size_t BufferCursor::getTailCol() const {
  return tailPosition.col;
}
