#include <fstream>
#include <iostream>
#include "const.hh"
#include "pane.hh"

EditBuffer::EditBuffer() {}
EditBuffer::EditBuffer(std::vector<std::string>&& l) : lines{l} {
  if (lines.size() == 0) {
    lines.push_back("");
  }
}
void EditBuffer::insertAtCursor(BufferCursor& cursor, int keycode) {
  if (lines.size() == 0) {
    lines.push_back("");
  }
  bool isSelection = cursor.getPosition() != cursor.getTailPosition();
  if (isSelection)
    clearSelection(cursor);
  switch (keycode) {
    case CARRIAGE_RETURN:
      carriageReturnAtCursor(cursor);
      break;
    case BACKSPACE:
      if (!isSelection)
        backspaceAtCursor(cursor);
      break;
    case DELETE:
      if (!isSelection)
        deleteAtCursor(cursor);
      break;
    case TAB:
      tabAtCursor(cursor);
      break;
    default:
      insertCharAtCursor(cursor, keycode);
      break;
  }
}

void EditBuffer::loadFromFile(const std::string& filename) {
  std::ifstream ifile{filename.c_str()};
  if (!ifile.is_open()) {
    std::cout << "ERROR:loadFile Failed to open: " << filename << std::endl;
    exitNed(1);
  }
  lines.clear();
  std::string line{""};
  while (std::getline(ifile, line)) {
    lines.push_back(line);
    line = "";
  }
  ifile.close();
}
void EditBuffer::insertCharAtCursor(BufferCursor& cursor, int keycode) {
  int cRow = cursor.getRow();
  int cCol = cursor.getCol();
  if (cCol >= (int)lines[cRow].size()) {
    lines[cRow].push_back((char)keycode);
  } else {
    lines[cRow].insert(lines[cRow].begin() + cCol, (char)keycode);
  }
  cursor.moveSet(cCol + 1, cRow);
}

void EditBuffer::carriageReturnAtCursor(BufferCursor& cursor) {
  int cRow = cursor.getRow();
  int cCol = cursor.getCol();
  std::string newLineText = "";
  if (cCol < (int)lines[cRow].size()) {
    // transfer the text after cursor on current line to new line
    newLineText = lines[cRow].substr(cCol);
    lines[cRow].erase(lines[cRow].begin() + cCol, lines[cRow].end());
  }
  if (cRow >= (int)lines.size() - 1) {
    // we are on the last row so we add a new line to the back
    lines.push_back(newLineText);
  } else {
    lines.insert(lines.begin() + cRow + 1, newLineText);
  }
  cursor.moveSet(0, cRow + 1);
}
void EditBuffer::backspaceAtCursor(BufferCursor& cursor) {
  // ignore selections for now;
  int cRow = cursor.getRow();
  int cCol = cursor.getCol();
  if (cCol == 0) {
    if (cRow == 0)
      return;
    // Cursor is at beginning of row, rows must be merged
    size_t preRowLength = lines[cRow - 1].size();
    // append line to previous line
    lines[cRow - 1].append(lines[cRow]);
    // remove line
    lines.erase(lines.begin() + cRow);
    cursor.moveSet(preRowLength, cRow - 1);
  } else {
    // Cursor is somewhere within row, just remove the previous character
    if (cCol > (int)lines[cRow].size()) {
      // if cursor is past end of line, go back to end of line
      cCol = lines[cRow].size();
    }
    // remove the character
    lines[cRow].erase(lines[cRow].begin() + cCol - 1);
    // move cursor back
    cursor.moveSet(cCol - 1, cRow);
  }
}
void EditBuffer::deleteAtCursor(const BufferCursor& cursor) {
  int cRow = cursor.getRow();
  int cCol = cursor.getCol();
  if (cRow == (int)lines.size() - 1 && cCol >= (int)lines[cRow].size()) {
    return;
  }
  if (cCol >= (int)lines[cRow].size()) {
    // Cursor is at end of row, rows must be merged
    lines[cRow].append(lines[cRow + 1]);
    lines.erase(lines.begin() + cRow + 1);
  } else {
    // Cursor is somewhere within row, just remove the character
    lines[cRow].erase(lines[cRow].begin() + cCol);
  }
}
void EditBuffer::tabAtCursor(BufferCursor& cursor) {
  int cRow = cursor.getRow();
  int cCol = cursor.getCol();
  const static std::string tab = "\t";
  if (cCol >= (int)lines[cRow].size()) {
    lines[cRow].append(tab);
  } else {
    std::string end = lines[cRow].substr(cCol, (int)lines[cRow].size() - cCol);
    lines[cRow].resize(lines[cRow].size() + tab.size());
    lines[cRow].replace(cCol, tab.size(), tab);
    lines[cRow].replace(cCol + tab.size(), end.size(), end);
  }
  cursor.moveSet(cCol + tab.size(), cRow);
}

void EditBuffer::clearSelection(BufferCursor& cursor) {
  BufferPosition start =
      std::min(cursor.getPosition(), cursor.getTailPosition());
  BufferPosition end = std::max(cursor.getPosition(), cursor.getTailPosition());
  if (start == end)
    return;
  if (start.col > lines[start.row].size()) {
    start.col = lines[start.row].size();
  }
  if (end.col > lines[end.row].size()) {
    end.col = lines[end.row].size();
  }
  if (start.row == end.row) {
    // single row
    lines[start.row].erase(lines[start.row].begin() + start.col,
                           lines[start.row].begin() + end.col);
  } else {
    // multi-row
    std::string tailString = lines[end.row].substr(end.col);
    lines[start.row].replace(start.col, lines[start.row].size() - start.col,
                             tailString);
    lines.erase(lines.begin() + start.row + 1, lines.begin() + end.row + 1);
  }
  cursor.moveSet(start.col, start.row);
}
