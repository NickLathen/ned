#include <iostream>
#include "pane.hh"

BufferCursor::BufferCursor() {}
BufferCursor::BufferCursor(BufferPosition& pos) : position{pos} {}

void BufferCursor::moveSet(int col, int row) {
  std::cout << "moveset: " << col << ", " << row << std::endl;
  position.row = row;
  position.col = col;
  tailPosition = position;
}
void BufferCursor::selectSet(int col, int row) {
  std::cout << "selectSet: " << col << ", " << row << std::endl;
  position.row = row;
  position.col = col;
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
  if (newY > (int)buf.lines.size() - 1) {
    newY = buf.lines.size() - 1;
    newX = buf.lines[newY].size();
  }
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
void BufferCursor::selectPageUp(int termHeight) {
  int nextRow = (int)position.row - termHeight / 2;
  if (nextRow < 0) {
    selectSet(0, 0);
  } else {
    selectSet(position.col, nextRow);
  }
}
void BufferCursor::selectPageDown(const EditBuffer& buf, int termHeight) {
  int bufSize = buf.lines.size();
  if (bufSize == 0)
    return;
  int nextRow = position.row + termHeight / 2;
  if (nextRow > bufSize - 1) {
    selectSet(buf.lines[bufSize - 1].size(), bufSize - 1);
  } else {
    selectSet(position.col, nextRow);
  }
}
void BufferCursor::selectHome() {
  selectSet(0, position.row);
}
void BufferCursor::selectEnd(const EditBuffer& buf) {
  selectSet(buf.lines[position.row].size(), position.row);
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
void BufferCursor::movePageUp(int termHeight) {
  selectPageUp(termHeight);
  tailPosition = position;
}
void BufferCursor::movePageDown(const EditBuffer& buf, int termHeight) {
  selectPageDown(buf, termHeight);
  tailPosition = position;
}
void BufferCursor::moveHome() {
  selectHome();
  tailPosition = position;
}
void BufferCursor::moveEnd(const EditBuffer& buf) {
  selectEnd(buf);
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
