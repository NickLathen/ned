#pragma once
#include <ncurses.h>
#include <string>
#include <vector>

enum PALETTES { N_TEXT = 1, N_GUTTER, N_INFO, N_COMMAND };

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
  std::vector<std::string> lines{};
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
  void moveRight(const EditBuffer& buf);
  void moveLeft(const EditBuffer& buf);
  void moveUp(const EditBuffer& buf);
  void moveDown(const EditBuffer& buf);
  void moveSet(int x, int y);
  void drawOffset(WINDOW* window,
                  const EditBuffer& buf,
                  const BufferPosition& bufOffset);
  BufferPosition position;

 private:
};

class Pane {
 public:
  Pane();
  Pane(WINDOW* window);
  Pane(WINDOW* window, EditBuffer&& eb);
  void addCursor();
  int getKeypress();
  void handleKeypress(int keycode);
  void loadFromFile(std::string filename);
  void redraw();

 private:
  WINDOW* window;
  std::string filename;
  EditBuffer buf;
  BufferPosition bufOffset;
  std::vector<BufferCursor> cursors;
  int getGutterWidth();
  void adjustOffsetToCursor(const BufferCursor& cursor);
  void adjustOffset();
  void setOffset(size_t row, size_t col);
  void drawBuffer();
  void drawCursors();
  void refresh();
  void erase();
};
