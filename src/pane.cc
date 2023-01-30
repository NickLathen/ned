#include "pane.hh"
#include <ncurses.h>
#include <panel.h>
#include <iostream>
#include <memory>
#include "const.hh"

int getNumDigits(int num) {
  int nums = 1;
  if (num == 0) {
    return 1;
  }
  num = num - (num % 10);
  int test = 1;
  while (num > test) {
    nums++;
    test *= 10;
  }
  return nums;
}

Pane::Pane(){};
Pane::Pane(WINDOW* window) : window{window} {};
Pane::Pane(WINDOW* window, EditBuffer&& eb) : window{window}, buf{eb} {};
void Pane::addCursor() {
  cursors.push_back(BufferCursor());
}
void Pane::handleKeypress(int keycode) {
  bool isHandledPress = false;
  switch (keycode) {
    case KEY_UP:
      isHandledPress = true;
      for (BufferCursor& c : cursors) {
        c.moveUp(buf);
      }
      break;
    case KEY_DOWN:
      isHandledPress = true;
      for (BufferCursor& c : cursors) {
        c.moveDown(buf);
      }
      break;
    case KEY_LEFT:
      isHandledPress = true;
      for (BufferCursor& c : cursors) {
        c.moveLeft(buf);
      }
      break;
    case KEY_RIGHT:
      isHandledPress = true;
      for (BufferCursor& c : cursors) {
        c.moveRight(buf);
      }
      break;
    default:
      if ((keycode >= 32 && keycode <= 126) || keycode == CARRIAGE_RETURN ||
          keycode == BACKSPACE || keycode == DELETE) {
        isHandledPress = true;
        // the keycode is printable or a carriage return
        for (BufferCursor& c : cursors) {
          buf.insertAtCursor(c, keycode);
        }
      }
      break;
  }
  if (isHandledPress) {
    redraw();
  }
}
void Pane::drawCursors() {
  int gutterWidth = getGutterWidth();
  BufferPosition bufOffsetWithGutter = bufOffset;
  bufOffsetWithGutter.col += gutterWidth;
  std::cout << "bufOffsetWithGutter{row, col}=" << bufOffsetWithGutter.row
            << ", " << bufOffsetWithGutter.col << std::endl;
  for (BufferCursor& c : cursors) {
    c.drawOffset(window, buf, bufOffsetWithGutter);
  }
}

int Pane::getGutterWidth() {
  return getNumDigits(buf.lines.size()) + 1;
}

void Pane::drawBlankLine(int row, int maxX, PALETTES color) {
  wmove(window, row, 0);
  wattron(window, COLOR_PAIR(color));
  for (int col = 0; col < maxX; col++)
    waddch(window, ' ');
}
void Pane::drawGutter(int row, int lineNumber, int gutterWidth) {
  wmove(window, row, 0);
  char lineNumBuf[16];
  int digits = std::snprintf(lineNumBuf, 16, "%d", lineNumber + 1);
  wattron(window, COLOR_PAIR(N_GUTTER));
  for (int col = 0; col < gutterWidth - 1; col++) {
    int digit = gutterWidth - col - 2;
    if (digit > digits - 1) {
      waddch(window, ' ');
      continue;
    }
    waddch(window, lineNumBuf[digits - 1 - digit]);
  }
  waddch(window, ' ');
}
void Pane::drawLine(int lineNumber, int startCol, int sz, PALETTES color) {
  std::string line = buf.lines[lineNumber];
  wattron(window, COLOR_PAIR(color));
  for (int i = startCol; i < sz; i++) {
    if (i >= (int)line.size()) {
      waddch(window, ' ');
    } else {
      waddch(window, line.at(i));
    }
  }
}
void Pane::drawInfoRow(int maxX, int maxY) {
  int cursorRow = cursors[0].position.row;
  int cursorCol = cursors[0].position.col;
  const char* filename_cstr = filename.c_str();
  int infoSz = std::snprintf(nullptr, 0, "%s (%d, %d)", filename_cstr,
                             cursorRow, cursorCol) +
               1;
  if (infoSz > maxX)
    infoSz = maxX;
  std::unique_ptr<char[]> infoBuf(new char[infoSz]);
  std::snprintf(infoBuf.get(), infoSz, "%s (%d, %d)", filename_cstr, cursorRow,
                cursorCol);
  wattron(window, COLOR_PAIR(N_INFO));
  wmove(window, maxY - 2, 0);
  for (int col = 0; col < maxX; col++) {
    if (col < infoSz - 1) {
      waddch(window, infoBuf.get()[col]);
    } else {
      waddch(window, ' ');
    }
  }
}

void Pane::drawBuffer() {
  if (buf.lines.size() == 0)
    return;
  int gutterWidth = getGutterWidth();
  int maxX, maxY;
  getmaxyx(window, maxY, maxX);
  for (int row = 0; row < maxY - 2; row++) {
    int lineNumber = row + bufOffset.row;
    if (lineNumber >= (int)buf.lines.size()) {
      drawBlankLine(row, maxX, N_TEXT);
      continue;
    }
    drawGutter(row, lineNumber, gutterWidth);
    drawLine(lineNumber, bufOffset.col, maxX - gutterWidth, N_TEXT);
  }
  drawInfoRow(maxX, maxY);
}
void Pane::adjustOffsetToCursor(const BufferCursor& cursor) {
  int maxX, maxY;
  getmaxyx(window, maxY, maxX);
  int bufX = cursor.position.col;
  int bufY = cursor.position.row;
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
  if (screenY < 0) {
    bufOffset.row = bufY;
  } else if (screenY >= maxY - 2) {
    bufOffset.row = bufY - maxY + 3;
  }
}
void Pane::adjustOffset() {
  if (cursors.size() == 0)
    return;
  adjustOffsetToCursor(cursors[0]);
}

void Pane::setOffset(size_t row, size_t col) {
  bufOffset.row = row;
  bufOffset.col = col;
}

void Pane::refresh() {
  update_panels();
  doupdate();
}
void Pane::erase() {
  werase(window);
}

int Pane::getKeypress() {
  return wgetch(window);
}

void Pane::loadFromFile(const std::string& iFilename) {
  buf.loadFromFile(iFilename);
  filename = iFilename;
}

void Pane::redraw() {
  adjustOffset();
  drawBuffer();
  drawCursors();
  refresh();
}
