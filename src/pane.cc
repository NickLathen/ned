#include "pane.hh"
#include <assert.h>
#include <ncurses.h>
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
enum Command { OPEN, SAVE, FIND };

Pane::Pane(WINDOW* window) : paneFocus{PF_TEXT}, window{window} {}

void Pane::addCursor() {
  cursors.push_back(BufferCursor{});
}
int Pane::getKeypress() const {
  return wgetch(window);
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
void Pane::initiateFindCommand() {
  paneFocus = PF_COMMAND;
  command = FIND;
  commandPrompt = "Find: ";
  userCommandArgs = "";
  commandCursorPosition = 0;
  redraw();
}
void Pane::saveBufferToFile(const std::string& saveTarget) const {
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
void Pane::handleSearch() {
  if (searchResults.isValid && searchResults.results.size() > 0) {
    // if we already have results, increment the index
    searchResults.index += 1;
    searchResults.index %= searchResults.results.size();
  } else {
    searchResults.results = getMatches(userCommandArgs);
    searchResults.index = 0;
    searchResults.isValid = true;
  }
  if (searchResults.results.size() > 0) {
    cursors.clear();
    cursors.push_back(searchResults.results[searchResults.index]);
  }
}
void Pane::saveBufOp(BufferOperation& bufOp) {
  if (opStackPosition < (int)opStack.size()) {
    opStack.erase(opStack.begin() + opStackPosition, opStack.end());
  }
  opStack.push_back(std::move(bufOp));
  opStackPosition++;
}
void Pane::undoLastBufOp() {
  if (opStackPosition > 0) {
    opStackPosition--;
    cursors = opStack[opStackPosition].iCursors;
    buf.undoBufferOperation(opStack[opStackPosition]);
  }
}
void Pane::redoNextBufOp() {
  if (opStackPosition < (int)opStack.size()) {
    cursors = opStack[opStackPosition].oCursors;
    BufferOperation bufCopy = opStack[opStackPosition];
    bufCopy.oCursors.clear();
    bufCopy.removedTexts.clear();
    buf.doBufferOperation(
        bufCopy);  // use a copy so we ignore changes to ocursors/removedTexts
    opStackPosition++;
  }
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
        case FIND:
          handleSearch();
          break;
      }
      break;
    case ESCAPE:
      isHandledPress = true;
      commandPrompt = "";
      userCommandArgs = "";
      paneFocus = PF_TEXT;
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
  int maxX, maxY;
  getmaxyx(window, maxY, maxX);
  bool isHandledPress = false;
  switch (keycode) {
    case CTRL_S:
      initiateSaveCommand();
      return;
    case CTRL_O:
      initiateOpenCommand();
      return;
    case CTRL_D:
      initiateFindCommand();
      return;
    case ARROW_UP:
      isHandledPress = true;
      for (BufferCursor& c : cursors) {
        c.moveUp(buf);
      }
      break;
    case ARROW_DOWN:
      isHandledPress = true;
      for (BufferCursor& c : cursors) {
        c.moveDown(buf);
      }
      break;
    case ARROW_LEFT:
      isHandledPress = true;
      for (BufferCursor& c : cursors) {
        c.moveLeft(buf);
      }
      break;
    case ARROW_RIGHT:
      isHandledPress = true;
      for (BufferCursor& c : cursors) {
        c.moveRight(buf);
      }
      break;
    case SHIFT_UP:
      isHandledPress = true;
      for (BufferCursor& c : cursors) {
        c.selectUp(buf);
      }
      break;
    case SHIFT_DOWN:
      isHandledPress = true;
      for (BufferCursor& c : cursors) {
        c.selectDown(buf);
      }
      break;
    case SHIFT_LEFT:
      isHandledPress = true;
      for (BufferCursor& c : cursors) {
        c.selectLeft(buf);
      }
      break;
    case SHIFT_RIGHT:
      isHandledPress = true;
      for (BufferCursor& c : cursors) {
        c.selectRight(buf);
      }
      break;
    case PAGE_UP:
      isHandledPress = true;
      for (BufferCursor& c : cursors) {
        c.movePageUp(maxY);
      }
      break;
    case PAGE_DOWN:
      isHandledPress = true;
      for (BufferCursor& c : cursors) {
        c.movePageDown(buf, maxY);
      }
      break;
    case HOME:
      isHandledPress = true;
      for (BufferCursor& c : cursors) {
        c.moveHome();
      }
      break;
    case END:
      isHandledPress = true;
      for (BufferCursor& c : cursors) {
        c.moveEnd(buf);
      }
      break;
    case SHIFT_PAGE_UP:
      isHandledPress = true;
      for (BufferCursor& c : cursors) {
        c.selectPageUp(maxY);
      }
      break;
    case SHIFT_PAGE_DOWN:
      isHandledPress = true;
      for (BufferCursor& c : cursors) {
        c.selectPageDown(buf, maxY);
      }
      break;
    case SHIFT_HOME:
      isHandledPress = true;
      for (BufferCursor& c : cursors) {
        c.selectHome();
      }
      break;
    case SHIFT_END:
      isHandledPress = true;
      for (BufferCursor& c : cursors) {
        c.selectEnd(buf);
      }
      break;
    case CTRL_Z:
      isHandledPress = true;
      undoLastBufOp();
      break;
    case CTRL_Y:
      isHandledPress = true;
      redoNextBufOp();
      break;
    default:
      if ((keycode >= 32 && keycode <= 126) || keycode == CARRIAGE_RETURN ||
          keycode == BACKSPACE || keycode == DELETE || keycode == TAB ||
          keycode == CTRL_UP || keycode == CTRL_DOWN) {
        isHandledPress = true;
        BufferOperation bufOp = buf.insertAtCursors(cursors, keycode);
        saveBufOp(bufOp);
      }
      break;
  }
  if (isHandledPress) {
    redraw();
  }
}

void Pane::adjustOffsetToCursor(const BufferCursor& cursor) {
  constexpr int xPad = 4;
  constexpr int yPad = 3;
  constexpr int infoHeight = 2;
  int maxX, maxY;
  getmaxyx(window, maxY, maxX);
  int bufX = cursor.getCol();
  int bufY = cursor.getRow();
  if (bufX > (int)buf.lines[bufY].size()) {
    bufX = buf.lines[bufY].size();
  }
  int gutterWidth = getGutterWidth();
  const std::string& line = buf.lines[bufY];
  int lIndex = 0;
  int screenX = 0;
  while (lIndex < bufX) {
    if (line[lIndex] != '\t') {
      screenX += 1;
    } else {
      int tabWidth = TABSTOPWIDTH - (screenX % TABSTOPWIDTH);
      screenX += tabWidth;
    }
    lIndex++;
  }
  screenX += gutterWidth - bufOffset.col;
  int screenY = bufY - bufOffset.row;
  if (screenX < xPad + gutterWidth) {
    bufOffset.col = std::max(bufX - xPad, 0);
  } else if (screenX >= maxX - xPad) {
    bufOffset.col = bufX - maxX + 1 + xPad + gutterWidth;
  }
  if (screenY < yPad) {
    bufOffset.row = std::max(0, bufY - yPad);
  } else if (screenY >= maxY - infoHeight - yPad) {
    bufOffset.row = bufY - maxY + yPad + infoHeight + 1;
  }
}
void Pane::adjustOffset() {
  if (cursors.size() == 0)
    return;
  adjustOffsetToCursor(getLeadCursor());
}

void Pane::drawBlankLine(int row, int maxX, PALETTES color) const {
  wmove(window, row, 0);
  wattron(window, COLOR_PAIR(color));
  for (int col = 0; col < maxX; col++)
    waddch(window, ' ');
}
void Pane::drawGutter(int row, int lineNumber, int gutterWidth) const {
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
void Pane::drawLine(int lineNumber, int startCol, int sz) const {
  wattron(window, COLOR_PAIR(N_TEXT));
  const std::string& line = buf.lines[lineNumber];
  int lIndex = 0;
  int screenX = 0;
  // find the first character lIndex in line that is after startCol, accounting
  // for tabs
  while (screenX < startCol) {
    if (line[lIndex] != '\t') {
      screenX += 1;
    } else {
      int tabWidth = TABSTOPWIDTH - (screenX % TABSTOPWIDTH);
      screenX += tabWidth;
    }
    lIndex++;
  }
  // if we overshot, add spaces from the tab that starts offscreen to left
  while (screenX > startCol) {
    waddch(window, ' ');
    screenX--;
  }
  // for every cell in the terminal, add the appropriate character
  for (int i = 0; i < sz; i++) {
    if (lIndex >= (int)line.size()) {
      waddch(window, ' ');
    } else if (line[lIndex] != '\t') {
      waddch(window, line.at(lIndex));
    } else {
      int tabWidth = TABSTOPWIDTH - ((i + startCol) % TABSTOPWIDTH);
      for (int j = 0; j < tabWidth; j++) {
        waddch(window, ' ');
      }
      i += (tabWidth - 1);
    }
    lIndex++;
  }
}
void Pane::drawInfoRow(int maxX, int maxY) const {
  BufferCursor leadCursor = getLeadCursor();
  int cursorRow = leadCursor.getRow();
  int cursorCol = leadCursor.getCol();
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
void Pane::drawCommandRow(int maxX, int maxY) const {
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
    mvwchgat(window, maxY - 1, cursorScreenX - offset, 1, A_STANDOUT, N_COMMAND,
             nullptr);
  }
}
void Pane::drawBuffer() const {
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
    drawLine(lineNumber, bufOffset.col, maxX - gutterWidth);
  }
  drawInfoRow(maxX, maxY);
  drawCommandRow(maxX, maxY);
}

void Pane::drawSingleCursor(const BufferCursor& cursor, int gutterWidth) const {
  int maxX, maxY;
  getmaxyx(window, maxY, maxX);
  int bufX = cursor.getCol();
  int bufY = cursor.getRow();
  int screenY = bufY - bufOffset.row;
  if (screenY < 0 || screenY >= maxY - 2)
    return;
  if (bufX > (int)buf.lines[bufY].size()) {
    bufX = buf.lines[bufY].size();
  }
  const std::string& line = buf.lines[bufY];
  int lIndex = 0;
  int screenX = 0;
  while (lIndex < bufX) {
    if (line[lIndex] != '\t') {
      screenX += 1;
    } else {
      int tabWidth = TABSTOPWIDTH - (screenX % TABSTOPWIDTH);
      screenX += tabWidth;
    }
    int screenPos = screenX + gutterWidth - bufOffset.col;
    if (screenPos >= maxX)
      return;  // cursor is off screen to right
    lIndex++;
  }
  screenX += gutterWidth - bufOffset.col;
  if (screenX < gutterWidth || screenX >= maxX)
    return;
  bool isSelection = cursor.getPosition() != cursor.getTailPosition();
  if (isSelection) {
    bool isSelectionTail = cursor.getPosition() > cursor.getTailPosition();
    if (isSelectionTail) {
      mvwchgat(window, screenY, screenX, 1, A_UNDERLINE, N_TEXT, nullptr);
    } else {
      mvwchgat(window, screenY, screenX, 1, A_UNDERLINE, N_HIGHLIGHT, nullptr);
    }
  } else {
    mvwchgat(window, screenY, screenX, 1, A_STANDOUT, N_TEXT, nullptr);
  }
}
void Pane::drawSelectionCursor(const BufferCursor& cursor) const {
  wattron(window, COLOR_PAIR(N_HIGHLIGHT));
  int maxX, maxY;
  getmaxyx(window, maxY, maxX);
  BufferPosition start =
      std::min(cursor.getPosition(), cursor.getTailPosition());
  BufferPosition end = std::max(cursor.getPosition(), cursor.getTailPosition());
  int gutterWidth = getGutterWidth();
  for (size_t row = start.row; row <= end.row; row++) {
    int screenY = row - bufOffset.row;
    if (screenY < 0 || screenY >= maxY - 2)
      continue;  // row is offscreen
    int selStartCol, selEndCol;
    if (start.row < row) {
      selStartCol = 0;
    } else if (start.row == row) {
      selStartCol = start.col;
    } else {
      assert(false);
    }
    if (end.row > row) {
      selEndCol = buf.lines[row].size();
    } else if (end.row == row) {
      selEndCol = end.col - 1;
    } else {
      assert(false);
    }
    if (selEndCol < 0)
      continue;  // selection ends at very beginning of line
    if (selEndCol > (int)buf.lines[row].size()) {
      selEndCol = buf.lines[row].size();
    }

    int distance = 0;
    int startDistance = -1;
    int endDistance = -1;
    for (int i = 0; i < (int)buf.lines[row].size(); i++) {
      if (i == selStartCol)
        startDistance = distance;
      if (i == selEndCol)
        endDistance = distance;
      if (startDistance >= 0 && endDistance >= 0)
        break;
      if (buf.lines[row][i] == '\t') {
        int tabWidth = TABSTOPWIDTH - (distance % TABSTOPWIDTH);
        distance += tabWidth;
      } else {
        distance += 1;
      }
      int screenDistance = distance + gutterWidth - bufOffset.col;
      if (screenDistance >= maxX) {
        // if we are past the end of the screen, don't bother finding the exact
        // distance
        break;
      }
    }

    if (endDistance == -1)
      endDistance = distance;
    if (startDistance == -1)
      startDistance = distance;
    int screenStartX = startDistance + gutterWidth - bufOffset.col;
    int screenEndX = endDistance + gutterWidth - bufOffset.col;
    if (screenStartX > maxX - 1)
      continue;  // starts offscreen to right
    if (screenEndX < gutterWidth)
      continue;  // ends offscreen to left
    if (screenStartX < gutterWidth)
      screenStartX = gutterWidth;
    if (screenEndX > maxX - 1)
      screenEndX = maxX - 1;
    mvwchgat(window, screenY, screenStartX, screenEndX - screenStartX + 1, 0,
             N_HIGHLIGHT, nullptr);
  }
}
void Pane::drawCursors() const {
  for (const BufferCursor& cursor : cursors) {
    int gutterWidth = getGutterWidth();
    if (cursor.getPosition() != cursor.getTailPosition()) {
      drawSelectionCursor(cursor);
    }
    drawSingleCursor(cursor, gutterWidth);
  }
}

std::vector<BufferCursor> Pane::getMatches(const std::string& query) const {
  std::vector<BufferCursor> result{};
  for (size_t row = 0; row < buf.lines.size(); row++) {
    const std::string& line = buf.lines[row];
    size_t start = 0;
    size_t match = 0;
    while ((match = line.find(query, start)) != std::string::npos) {
      BufferCursor bc{};
      start = match + query.size();
      bc.moveSet(match, row);
      bc.selectSet(start, row);
      result.push_back(bc);
    }
  }
  return result;
}

void Pane::refresh() const {
  doupdate();
}
void Pane::erase() const {
  werase(window);
}
int Pane::getGutterWidth() const {
  return getNumDigits(buf.lines.size()) + 1;
}
BufferCursor Pane::getLeadCursor() const {
  return cursors[cursors.size() - 1];
}
