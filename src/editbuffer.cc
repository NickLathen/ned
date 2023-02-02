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
    case CTRL_UP:
      bo.opType = BO_SLIDE_UP;
      break;
    case CTRL_DOWN:
      bo.opType = BO_SLIDE_DOWN;
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
    switch (bufOp.opType) {
      case BO_INSERT:
        clearSelection(targetCursor);
        insertTextAtCursor(targetCursor, bufOp.insertText);
        break;
      case BO_BACKSPACE:
        clearSelection(targetCursor);
        if (!isSelection)
          backspaceAtCursor(targetCursor);
        break;
      case BO_DELETE:
        clearSelection(targetCursor);
        if (!isSelection)
          deleteAtCursor(targetCursor);
        break;
      case BO_SLIDE_UP:
        slideUpAtCursor(targetCursor);
        break;
      case BO_SLIDE_DOWN:
        slideDownAtCursor(targetCursor);
        break;
    }
    bufOp.oCursors.push_back(targetCursor);
  }
}

void EditBuffer::slideUpAtCursor(BufferCursor& cursor) {
  BufferPosition a = cursor.getPosition();
  BufferPosition b = cursor.getTailPosition();
  BufferPosition start = std::min(a, b);
  BufferPosition end = std::max(a, b);
  if (start.row <= 0)
    return;
  // copy the line above
  std::string preLine = "\n" + lines[start.row - 1];
  // delete the line above
  lines.erase(lines.begin() + start.row - 1);
  // insert the copy at the end of the last line of the selection
  BufferPosition insertPos{end.row - 1, lines[end.row - 1].size()};
  BufferCursor insertCursor{insertPos};
  insertTextAtCursor(insertCursor, preLine);
  // move cursor up a row
  cursor.moveSet(b.col, b.row - 1);
  cursor.selectSet(a.col, a.row - 1);
}
void EditBuffer::slideDownAtCursor(BufferCursor& cursor) {
  BufferPosition a = cursor.getPosition();
  BufferPosition b = cursor.getTailPosition();
  BufferPosition start = std::min(a, b);
  BufferPosition end = std::max(a, b);
  if (end.row >= lines.size() - 1)
    return;
  // copy the line below
  std::string postLine = lines[end.row + 1] + "\n";
  // delete the line below
  lines.erase(lines.begin() + end.row + 1);
  // insert copy at beginning of line of first line of selection
  BufferPosition insertPos{start.row, 0};
  BufferCursor insertCursor{insertPos};
  insertTextAtCursor(insertCursor, postLine);
  // move cursor down
  cursor.moveSet(b.col, b.row + 1);
  cursor.selectSet(a.col, a.row + 1);
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
