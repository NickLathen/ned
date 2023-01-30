#include "pane.hh"
#include <ncurses.h>
#include <panel.h>
#include <fstream>
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

enum PaneFocus { PF_TEXT, PF_COMMAND };
enum Command { OPEN, SAVE };

Pane::Pane() : paneFocus{PF_TEXT} {}
Pane::Pane(WINDOW* window) : paneFocus{PF_TEXT}, window{window} {}
Pane::Pane(WINDOW* window, EditBuffer&& eb)
    : paneFocus{PF_TEXT}, window{window}, buf{eb} {}
void Pane::addCursor() {
  cursors.push_back(BufferCursor{});
}
void Pane::handleKeypress(int keycode) {
  switch (paneFocus) {
    case PF_TEXT:
      handleTextKeypress(keycode);
      break;
    case PF_COMMAND:
      handleCommandKeypress(keycode);
      break;
    default:
      break;
  }
}
void Pane::initiateSaveCommand() {
  paneFocus = PF_COMMAND;
  command = SAVE;
  commandPrompt = "Save Filename: ";
  userCommandArgs = filename;
  commandCursorPosition = filename.size();
  redraw();
}
void Pane::initiateOpenCommand() {
  paneFocus = PF_COMMAND;
  command = OPEN;
  commandPrompt = "Open Filename: ";
  userCommandArgs = "";
  commandCursorPosition = 0;
  redraw();
}
void Pane::saveBufferToFile(std::string saveTarget) {
  std::ofstream saveFile{"ned.tmp", std::ios_base::trunc | std::ios_base::out};
  for (size_t line = 0; line < buf.lines.size(); line++) {
    for (size_t col = 0; col < buf.lines[line].size(); col++) {
      saveFile.put(buf.lines[line][col]);
    }
    if (line < buf.lines.size() - 1) {
      saveFile.put('\n');
    }
  }
  saveFile.close();
  std::rename(saveTarget.c_str(), "ned.tmp.bak");
  std::rename("ned.tmp", saveTarget.c_str());
  std::remove("ned.tmp.bak");
}
void Pane::handleCommandKeypress(int keycode) {
  bool isHandledPress = false;
  switch (keycode) {
    case CARRIAGE_RETURN:
      isHandledPress = true;
      switch (command) {
        case SAVE:
          saveBufferToFile(userCommandArgs);
          filename = userCommandArgs;
          commandPrompt = "Saved File";
          userCommandArgs = "";
          paneFocus = PF_TEXT;
          break;
        case OPEN:
          loadFromFile(userCommandArgs);
          commandPrompt = "Loaded File";
          userCommandArgs = "";
          paneFocus = PF_TEXT;
          break;
      }
      break;
    case BACKSPACE:
      if (commandCursorPosition > 0) {
        isHandledPress = true;
        userCommandArgs.erase(userCommandArgs.begin() + commandCursorPosition -
                              1);
        commandCursorPosition--;
      }
      break;
    case DELETE:
      if (commandCursorPosition < (int)userCommandArgs.size()) {
        isHandledPress = true;
        userCommandArgs.erase(userCommandArgs.begin() + commandCursorPosition);
      }
      break;
    default:
      isHandledPress = true;
      if (commandCursorPosition >= (int)userCommandArgs.size()) {
        userCommandArgs.push_back((char)keycode);
        commandCursorPosition = userCommandArgs.size();
      } else {
        userCommandArgs.insert(userCommandArgs.begin() + commandCursorPosition,
                               (char)keycode);
        commandCursorPosition++;
      }
      break;
  }
  if (isHandledPress) {
    redraw();
  }
}
void Pane::handleTextKeypress(int keycode) {
  bool isHandledPress = false;
  switch (keycode) {
    case CTRL_S:
      initiateSaveCommand();
      return;
    case CTRL_O:
      initiateOpenCommand();
      return;
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
  for (BufferCursor& c : cursors) {
    int maxX, maxY;
    getmaxyx(window, maxY, maxX);
    int bufX = c.position.col;
    int bufY = c.position.row;
    if (bufX > (int)buf.lines[bufY].size()) {
      bufX = buf.lines[bufY].size();
    }
    int screenX = bufX - bufOffset.col + gutterWidth;
    int screenY = bufY - bufOffset.row;
    // ignore offscreen
    if (screenX < gutterWidth || screenX >= maxX || screenY < 0 ||
        screenY >= maxY - 2)
      return;

    mvwchgat(window, screenY, screenX, 1, A_STANDOUT, 0, nullptr);
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
  for (int i = startCol; i < startCol + sz; i++) {
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
void Pane::drawCommandRow(int maxX, int maxY) {
  size_t promptSize = commandPrompt.size();
  size_t argSize = userCommandArgs.size();
  size_t commandSize = promptSize + argSize;
  int cursorScreenX = promptSize + commandCursorPosition;
  int offset = 0;
  if (cursorScreenX > maxX) {
    offset = cursorScreenX - maxX;
  }
  // draw the text
  wattron(window, COLOR_PAIR(N_COMMAND));
  wmove(window, maxY - 1, 0);
  for (int col = 0; col < maxX; col++) {
    int bufIndex = col + offset;
    if (bufIndex <= (int)promptSize - 1) {
      waddch(window, commandPrompt[bufIndex]);
    } else if (bufIndex <= (int)commandSize - 1) {
      waddch(window, userCommandArgs[bufIndex - promptSize]);
    } else {
      waddch(window, ' ');
    }
  }
  // draw the cursor
  if (paneFocus == PF_COMMAND) {
    mvwchgat(window, maxY - 1, cursorScreenX - offset, 1, A_STANDOUT,
             COLOR_PAIR(N_COMMAND), nullptr);
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
  drawCommandRow(maxX, maxY);
}
void Pane::adjustOffsetToCursor(const BufferCursor& cursor) {
  int maxX, maxY;
  getmaxyx(window, maxY, maxX);
  int bufX = cursor.position.col;
  int bufY = cursor.position.row;
  if (bufX > (int)buf.lines[bufY].size()) {
    bufX = buf.lines[bufY].size();
  }
  int gutterWidth = getGutterWidth();
  int screenX = bufX - bufOffset.col + gutterWidth;
  int screenY = bufY - bufOffset.row;
  if (screenX < 4 + gutterWidth) {
    bufOffset.col = std::max(bufX - 4, 0);
  } else if (screenX >= maxX - 4) {
    bufOffset.col = bufX - maxX + 5 + gutterWidth;
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
  std::cout << "bufOffset{row,col}: {" << bufOffset.row << ", " << bufOffset.col
            << "}" << std::endl;
  drawBuffer();
  drawCursors();
  refresh();
}
