#include "log.hh"
#include "pane.hh"

BufferCursor::BufferCursor() : position{} {};
BufferCursor::BufferCursor(BufferPosition pos) : position{pos} {};
void BufferCursor::moveSet(int x, int y) {
  LOGF << "moveset: " << x << ", " << y << std::endl;
  position.row = y;
  position.col = x;
}
void BufferCursor::moveRight(const EditBuffer& buf) {
  if (buf.lines.size() == 0) {
    moveSet(0, 0);
    return;
  }
  int newX = position.col + 1;
  int newY = position.row;
  if (newY < 0)
    newY = 0;
  if (newY >= (int)buf.lines.size())
    newY = buf.lines.size() - 1;
  if (position.col < 0) {
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
void BufferCursor::moveLeft(const EditBuffer& buf) {
  if (buf.lines.size() == 0) {
    moveSet(0, 0);
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
void BufferCursor::moveUp(const EditBuffer& buf) {
  if (buf.lines.size() == 0) {
    moveSet(0, 0);
    return;
  }
  int newX = position.col;
  int newY = position.row - 1;
  if (newY < 0) {
    moveSet(0, 0);
    return;
  }
  if (newY >= (int)buf.lines.size() - 1)
    newY = std::max((int)buf.lines.size() - 2, 0);
  moveSet(newX, newY);
};
void BufferCursor::moveDown(const EditBuffer& buf) {
  if (buf.lines.size() == 0) {
    moveSet(0, 0);
    return;
  }
  int newX = position.col;
  int newY = position.row + 1;
  if (newY < 0) {
    newY = std::min(1, (int)buf.lines.size());
    moveSet(newX, newY);
    return;
  }
  if (newY > (int)buf.lines.size() - 1)
    newY = buf.lines.size() - 1;
  moveSet(newX, newY);
};
void BufferCursor::drawOffset(WINDOW* window,
                              const EditBuffer& buf,
                              const BufferPosition& bufOffset) {
  int maxX, maxY;
  getmaxyx(window, maxY, maxX);
  int bufX = position.col;
  int bufY = position.row;
  if (bufX > (int)buf.lines[bufY].size()) {
    bufX = buf.lines[bufY].size();
  }
  int screenX = bufX + bufOffset.col;
  int screenY = bufY - bufOffset.row;
  // ignore offscreen
  if (screenX < 0 || screenX >= maxX || screenY < 0 || screenY >= maxY)
    return;

  mvwchgat(window, screenY, screenX, 1, A_STANDOUT, 0, nullptr);
}
