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
  std::vector<BufferCursor> dummycursors{};
  std::string c{(char)keycode};
  BufferOperation bufOp(cursor, dummycursors, c);
  doBufferOperation(bufOp);
  cursor = bufOp.targetCursor;
}
void EditBuffer::doBufferOperation(BufferOperation& bufOp) {
  clearSelection(bufOp.targetCursor);
  insertTextAtCursor(bufOp.targetCursor, bufOp.insertText);
}
void EditBuffer::insertTextAtCursor(BufferCursor& cursor, std::string& text) {
  std::vector<std::string> insertLines{};
  std::string insertLine{""};
  for (int i = 0; i <= (int)text.size(); i++) {
    if (i == (int)text.size() || text[i] == '\n') {
      insertLines.push_back(insertLine);
      insertLine = "";
      continue;
    }
    insertLine.append(1, text.at(i));
  }
  int cRow = cursor.getRow();
  int cCol = cursor.getCol();
  if (cCol > (int)lines[cRow].size())
    cCol = lines[cRow].size();
  std::string preString = lines[cRow].substr(0, cCol);
  std::string postString = lines[cRow].substr(cCol);
  int lastLineLength = insertLines[insertLines.size() - 1].size();
  insertLines[0] = preString + insertLines[0];
  insertLines[insertLines.size() - 1] =
      insertLines[insertLines.size() - 1] + postString;
  lines.erase(lines.begin() + cRow);
  lines.insert(lines.begin() + cRow, insertLines.begin(), insertLines.end());
  cRow += insertLines.size() - 1;
  cCol = lastLineLength;
  if (insertLines.size() == 1) {
    cCol += preString.size();
  }
  cursor.moveSet(cCol, cRow);
}

void EditBuffer::carriageReturnAtCursor(BufferCursor& cursor) {
  std::vector<BufferCursor> dummycursors{};
  std::string c{'\n'};
  BufferOperation bufOp(cursor, dummycursors, c);
  doBufferOperation(bufOp);
  cursor = bufOp.targetCursor;
}
void EditBuffer::backspaceAtCursor(BufferCursor& cursor) {
  int cRow = cursor.getRow();
  int cCol = cursor.getCol();
  if (cCol >= (int)lines[cRow].size())
    cCol = lines[cRow].size();
  if (cRow == 0 && cCol == 0)
    return;
  if (cCol == 0) {
    cursor.selectSet(lines[cRow - 1].size(), cRow - 1);
  } else {
    cursor.selectSet(cCol - 1, cRow);
  }
  std::vector<BufferCursor> dummycursors{};
  std::string c{""};
  BufferOperation bufOp(cursor, dummycursors, c);
  doBufferOperation(bufOp);
  cursor = bufOp.targetCursor;
}
void EditBuffer::deleteAtCursor(BufferCursor& cursor) {
  int cRow = cursor.getRow();
  int cCol = cursor.getCol();
  if (cCol >= (int)lines[cRow].size())
    cCol = lines[cRow].size();
  if (cRow > (int)lines.size() - 1 ||
      (cRow == (int)lines.size() - 1 && cCol > (int)lines[cRow].size() - 1))
    return;
  if (cCol > (int)lines[cRow].size() - 1) {
    cursor.selectSet(0, cRow + 1);
  } else {
    cursor.selectSet(cCol + 1, cRow);
  }
  std::vector<BufferCursor> dummycursors{};
  std::string c{""};
  BufferOperation bufOp(cursor, dummycursors, c);
  doBufferOperation(bufOp);
  cursor = bufOp.targetCursor;
}
void EditBuffer::tabAtCursor(BufferCursor& cursor) {
  std::vector<BufferCursor> dummycursors{};
  std::string c{'\t'};
  BufferOperation bufOp(cursor, dummycursors, c);
  doBufferOperation(bufOp);
  cursor = bufOp.targetCursor;
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
