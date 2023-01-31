#pragma once
#include <ncurses.h>
#include <string>
#include <vector>

enum PALETTES { N_TEXT = 1, N_GUTTER, N_INFO, N_COMMAND, N_HIGHLIGHT };

class BufferCursor;

// points to a specific character in a buffer
struct BufferPosition {
  size_t row{}, col{};
};

// data from file
class EditBuffer {
 public:
  EditBuffer();
  EditBuffer(std::vector<std::string>&& lines);
  std::vector<std::string> lines{""};
  void insertAtCursor(BufferCursor& cursor, int keycode);
  void loadFromFile(std::string filename);

 private:
  void carriageReturnAtCursor(BufferCursor& cursor);
  void backspaceAtCursor(BufferCursor& cursor);
  void deleteAtCursor(const BufferCursor& cursor);
};

class BufferCursor {
 public:
  BufferCursor();
  BufferCursor(BufferPosition pos);
  void selectUp(const EditBuffer& buf);
  void selectDown(const EditBuffer& buf);
  void selectLeft(const EditBuffer& buf);
  void selectRight(const EditBuffer& buf);
  void moveUp(const EditBuffer& buf);
  void moveDown(const EditBuffer& buf);
  void moveLeft(const EditBuffer& buf);
  void moveRight(const EditBuffer& buf);

  void moveSet(int x, int y);
  void selectSet(int x, int y);
  BufferPosition position{};
  BufferPosition tailPosition{};
};
bool operator==(const BufferPosition& a, const BufferPosition& b);

class Pane {
 public:
  Pane();
  Pane(WINDOW* window);
  Pane(WINDOW* window, EditBuffer&& eb);
  void addCursor();
  int getKeypress();
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
  int getGutterWidth();
  void adjustOffsetToCursor(const BufferCursor& cursor);
  void adjustOffset();
  void saveBufferToFile(std::string saveTarget);
  void initiateSaveCommand();
  void initiateOpenCommand();
  void handleTextKeypress(int keycode);
  void handleCommandKeypress(int keycode);
  void setOffset(size_t row, size_t col);
  void drawBlankLine(int row, int maxX, PALETTES color);
  void drawGutter(int row, int lineNumber, int gutterWidth);
  void drawLine(int lineNumber, int startCol, int sz);
  void drawInfoRow(int maxX, int maxY);
  void drawCommandRow(int maxX, int maxY);
  void drawBuffer();
  void drawSingleCursor(const BufferCursor& cursor, int gutterWidth);
  void drawSelectionCursor(const BufferCursor& cursor);
  void drawCursors();
  void refresh();
  void erase();
};
