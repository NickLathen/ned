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
void EditBuffer::insertAtCursors(std::vector<BufferCursor>& cursors,
                                 int keycode) {
  if (lines.size() == 0) {
    lines.push_back("");
  }
  BufferOperation bo{cursors, "", BO_INSERT};
  switch (keycode) {
    case BACKSPACE:
      bo.opType = BO_BACKSPACE;
      break;
    case DELETE:
      bo.opType = BO_DELETE;
      break;
    case CARRIAGE_RETURN:
      bo.insertText.append(1, '\n');
      break;
    case TAB:
      bo.insertText.append(1, '\t');
      break;
    default:
      bo.insertText.append(1, keycode);
      break;
  }
  doBufferOperation(bo);
  cursors = bo.oCursors;
}
void EditBuffer::loadFromFile(const std::string& filename) {
  std::ifstream ifile{filename.c_str()};
  if (!ifile.is_open()) {
    std::cout << "ERROR:loadFile Failed to open: " << filename << std::endl;
    exitNed(1);
  }
  lines.clear();
  std::string line{};
  while (std::getline(ifile, line)) {
    lines.push_back(line);
    line = "";
  }
  ifile.close();
}
void EditBuffer::doBufferOperation(BufferOperation& bufOp) {
  for (auto targetCursor : bufOp.iCursors) {
    bool isSelection =
        targetCursor.getPosition() != targetCursor.getTailPosition();
    if (isSelection)
      clearSelection(targetCursor);
    switch (bufOp.opType) {
      case BO_INSERT:
        insertTextAtCursor(targetCursor, bufOp.insertText);
        break;
      case BO_BACKSPACE:
        if (!isSelection)
          backspaceAtCursor(targetCursor);
        break;
      case BO_DELETE:
        if (!isSelection)
          deleteAtCursor(targetCursor);
        break;
    }
    bufOp.oCursors.push_back(targetCursor);
  }
}

void EditBuffer::insertTextAtCursor(BufferCursor& cursor, std::string& text) {
  std::vector<std::string> insertLines{};
  std::string insertLine{};
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
  clearSelection(cursor);
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
  clearSelection(cursor);
}

void EditBuffer::clearSelection(BufferCursor& cursor) {
  BufferPosition a = cursor.getPosition();
  BufferPosition b = cursor.getTailPosition();
  if (a == b)
    return;
  BufferPosition start = std::min(a, b);
  BufferPosition end = std::max(a, b);
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
