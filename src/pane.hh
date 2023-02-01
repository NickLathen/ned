#pragma once
#include <ncurses.h>
#include <string>
#include <vector>

enum PALETTES { N_TEXT = 1, N_GUTTER, N_INFO, N_COMMAND, N_HIGHLIGHT };

class BufferCursor;
class BufferOperation;

// points to a specific character in a buffer
struct BufferPosition {
  size_t row{}, col{};
};
bool operator==(const BufferPosition&, const BufferPosition&);
bool operator!=(const BufferPosition&, const BufferPosition&);
bool operator<(const BufferPosition&, const BufferPosition&);
bool operator<=(const BufferPosition&, const BufferPosition&);
bool operator>(const BufferPosition&, const BufferPosition&);
bool operator>=(const BufferPosition&, const BufferPosition&);

// data from file
class EditBuffer {
 public:
  EditBuffer();
  EditBuffer(std::vector<std::string>&& lines);
  std::vector<std::string> lines{""};
  void insertAtCursors(std::vector<BufferCursor>& cursors, int keycode);
  void loadFromFile(const std::string& filename);

 private:
  void doBufferOperation(BufferOperation& bufOp);
  void insertTextAtCursor(BufferCursor& cursor, std::string& text);
  void backspaceAtCursor(BufferCursor& cursor);
  void deleteAtCursor(BufferCursor& cursor);
  void clearSelection(BufferCursor& cursor);
};

class BufferCursor {
 public:
  BufferCursor();
  BufferCursor(BufferPosition& pos);
  void moveSet(int col, int row);
  void selectSet(int col, int row);
  void selectUp(const EditBuffer& buf);
  void selectDown(const EditBuffer& buf);
  void selectLeft(const EditBuffer& buf);
  void selectRight(const EditBuffer& buf);
  void moveUp(const EditBuffer& buf);
  void moveDown(const EditBuffer& buf);
  void moveLeft(const EditBuffer& buf);
  void moveRight(const EditBuffer& buf);
  BufferPosition getPosition() const;
  BufferPosition getTailPosition() const;
  size_t getRow() const;
  size_t getCol() const;
  size_t getTailRow() const;
  size_t getTailCol() const;

 private:
  BufferPosition position{};
  BufferPosition tailPosition{};
};

enum BufOpType { BO_INSERT, BO_BACKSPACE, BO_DELETE };

class BufferOperation {
 public:
  BufferOperation(std::vector<BufferCursor>& cursors,
                  std::string&& text,
                  BufOpType ot);
  std::vector<BufferCursor> iCursors{};
  std::string insertText{""};
  BufOpType opType;
  std::vector<BufferCursor> oCursors{};
  std::string removedText{""};
};

class Pane {
 public:
  Pane();
  Pane(WINDOW* window);
  Pane(WINDOW* window, EditBuffer&& eb);
  void addCursor();
  int getKeypress() const;
  void handleKeypress(int keycode);
  void loadFromFile(const std::string& filename);
  void redraw();

 private:
  int paneFocus{};
  int command{};
  int commandCursorPosition{};
  WINDOW* window{};
  std::string filename{""};
  std::string commandPrompt{""};
  std::string userCommandArgs{""};
  EditBuffer buf{};
  BufferPosition bufOffset{};
  std::vector<BufferCursor> cursors{BufferCursor{}};
  void initiateSaveCommand();
  void initiateOpenCommand();
  void saveBufferToFile(const std::string& saveTarget) const;
  void handleCommandKeypress(int keycode);
  void handleTextKeypress(int keycode);

  void adjustOffsetToCursor(const BufferCursor& cursor);
  void adjustOffset();

  void drawBlankLine(int row, int maxX, PALETTES color) const;
  void drawGutter(int row, int lineNumber, int gutterWidth) const;
  void drawLine(int lineNumber, int startCol, int sz) const;
  void drawInfoRow(int maxX, int maxY) const;
  void drawCommandRow(int maxX, int maxY) const;
  void drawBuffer() const;

  void drawSingleCursor(const BufferCursor& cursor, int gutterWidth) const;
  void drawSelectionCursor(const BufferCursor& cursor) const;
  void drawCursors() const;

  void refresh() const;
  void erase() const;
  int getGutterWidth() const;
};
