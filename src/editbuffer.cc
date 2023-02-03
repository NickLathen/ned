#include <fstream>
#include <iostream>
#include "const.hh"
#include "pane.hh"

BufferOperation EditBuffer::insertAtCursors(std::vector<BufferCursor>& cursors,
                                            int keycode) {
  if (lines.size() == 0) {
    lines.push_back("");
  }
  std::string insertText{""};
  std::vector<std::string> insertTexts{};
  BufOpType bot = BO_INSERT;
  switch (keycode) {
    case BACKSPACE:
      bot = BO_BACKSPACE;
      break;
    case DELETE:
      bot = BO_DELETE;
      break;
    case CTRL_UP:
      bot = BO_SLIDE_UP;
      break;
    case CTRL_DOWN:
      bot = BO_SLIDE_DOWN;
      break;
    case CARRIAGE_RETURN:
      insertText.append(1, '\n');
      break;
    case TAB:
      insertText.append(1, '\t');
      break;
    default:
      insertText.append(1, keycode);
      break;
  }
  insertTexts.insert(insertTexts.begin(), cursors.size(), insertText);
  BufferOperation bufOp{bot, cursors, insertTexts};
  doBufferOperation(bufOp);
  cursors = bufOp.oCursors;
  return bufOp;
}
void EditBuffer::undoBufferOperation(const BufferOperation& bufOp) {
  switch (bufOp.opType) {
    case BO_INSERT:
      undoInsertText(bufOp);
      break;
    case BO_BACKSPACE:
      undoClearSelection(bufOp);
      break;
    case BO_DELETE:
      undoClearSelection(bufOp);
      break;
    case BO_SLIDE_UP:
      undoSlideUp(bufOp);
      break;
    case BO_SLIDE_DOWN:
      undoSlideDown(bufOp);
      break;
  }
}

BufferCursor EditBuffer::selectPrecedingText(const std::string& text,
                                             BufferCursor endCursor) {
  int numLines{}, firstLineLength{};
  for (size_t i = 0; i < text.size(); i++) {
    if (text[i] == '\n') {
      numLines++;
      continue;
    }
    if (numLines == 0)
      firstLineLength++;
  }
  int sRow = endCursor.getRow() - numLines;
  int sCol{};
  if (numLines == 0) {
    sCol = endCursor.getCol() - firstLineLength;
  } else {
    sCol = lines[sRow].size() - firstLineLength;
  }
  endCursor.selectSet(sCol, sRow);
  return endCursor;
}

void EditBuffer::undoInsertText(const BufferOperation& bufOp) {
  for (size_t i = 0; i < bufOp.insertTexts.size(); i++) {
    BufferCursor insertCursor =
        selectPrecedingText(bufOp.insertTexts[i], bufOp.oCursors[i]);
    // remove the inserted text
    clearSelection(insertCursor);
    // add the removed text
    insertTextAtCursor(insertCursor, bufOp.removedTexts[i]);
  }
}
void EditBuffer::undoClearSelection(const BufferOperation& bufOp) {
  for (size_t i = 0; i < bufOp.removedTexts.size(); i++) {
    BufferCursor insertCursor = bufOp.oCursors[i];
    insertTextAtCursor(insertCursor, bufOp.removedTexts[i]);
  }
}
void EditBuffer::undoSlideUp(const BufferOperation& bufOp) {
  BufferOperation uBufOp{BO_SLIDE_DOWN, bufOp.oCursors, {}};
  doBufferOperation(uBufOp);
}
void EditBuffer::undoSlideDown(const BufferOperation& bufOp) {
  BufferOperation uBufOp{BO_SLIDE_UP, bufOp.oCursors, {}};
  doBufferOperation(uBufOp);
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
  for (size_t i = 0; i < bufOp.iCursors.size(); i++) {
    BufferCursor cursor = bufOp.iCursors[i];
    std::string removedText{""};
    switch (bufOp.opType) {
      case BO_INSERT:
        removedText = clearSelection(cursor);
        insertTextAtCursor(cursor, bufOp.insertTexts[i]);
        break;
      case BO_BACKSPACE:
        removedText = clearSelection(cursor);
        if (removedText.size() == 0)
          backspaceAtCursor(cursor, removedText);
        break;
      case BO_DELETE:
        removedText = clearSelection(cursor);
        if (removedText.size() == 0)
          deleteAtCursor(cursor, removedText);
        break;
      case BO_SLIDE_UP:
        slideUpAtCursor(cursor);
        break;
      case BO_SLIDE_DOWN:
        slideDownAtCursor(cursor);
        break;
    }
    bufOp.oCursors.push_back(cursor);
    bufOp.removedTexts.push_back(removedText);
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
void EditBuffer::insertTextAtCursor(BufferCursor& cursor,
                                    const std::string& text) {
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
void EditBuffer::backspaceAtCursor(BufferCursor& cursor,
                                   std::string& removedText) {
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
  removedText = clearSelection(cursor);
}
void EditBuffer::deleteAtCursor(BufferCursor& cursor,
                                std::string& removedText) {
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
  removedText = clearSelection(cursor);
}
std::string EditBuffer::clearSelection(BufferCursor& cursor) {
  BufferPosition a = cursor.getPosition();
  BufferPosition b = cursor.getTailPosition();
  if (a == b)
    return "";
  std::string removedText{stringifySelection(cursor)};
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
  return removedText;
}
std::string EditBuffer::stringifySelection(BufferCursor& cursor) {
  std::string result{""};
  BufferPosition a = cursor.getPosition();
  BufferPosition b = cursor.getTailPosition();
  if (a == b)
    return result;
  BufferPosition start = std::min(a, b);
  BufferPosition end = std::max(a, b);
  for (int i = start.row; i <= (int)end.row; i++) {
    int lineSize = lines[i].size();
    int rowStart = i == (int)start.row ? start.col : 0;
    if (rowStart > lineSize)
      rowStart = lineSize;
    int rowEnd = i == (int)end.row ? end.col : lineSize;
    if (rowEnd > lineSize - 1)
      rowEnd = lineSize;
    result.append(lines[i].substr(rowStart, rowEnd - rowStart));
    if (i < (int)end.row)
      result.append("\n");
  }
  return result;
}
