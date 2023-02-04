#pragma once
#include <ncurses.h>
#include <string>
#include <vector>

enum PALETTES { N_TEXT = 1, N_GUTTER, N_INFO, N_COMMAND, N_HIGHLIGHT };

class BufferCursor;
class BufferOperation;

struct SearchResults {
  bool isValid{false};
  size_t index{0};
  std::vector<BufferCursor> results{};
};

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
  std::vector<std::string> lines{};
  BufferOperation insertAtCursors(std::vector<BufferCursor>& cursors,
                                  int keycode);
  void undoBufferOperation(const BufferOperation& bufOp);
  void loadFromFile(const std::string& filename);
  void doBufferOperation(BufferOperation& bufOp);

 private:
  void insertTextAtCursor(BufferCursor& cursor, const std::string& text);
  void backspaceAtCursor(BufferCursor& cursor, std::string& removedText);
  void deleteAtCursor(BufferCursor& cursor, std::string& removedText);
  void slideUpAtCursor(BufferCursor& cursor);
  void slideDownAtCursor(BufferCursor& cursor);
  BufferCursor selectPrecedingText(const std::string& insertText,
                                   BufferCursor insertCursor);
  void undoInsertText(const BufferOperation& bufOp);
  void undoClearSelection(const BufferOperation& bufOp);
  void undoSlideUp(const BufferOperation& bufOp);
  void undoSlideDown(const BufferOperation& bufOp);
  std::string clearSelection(BufferCursor& cursor);
  std::string stringifySelection(BufferCursor& cursor);
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
  void selectPageUp(int termHeight);
  void selectPageDown(const EditBuffer& buf, int termHeight);
  void selectHome();
  void selectEnd(const EditBuffer& buf);
  void moveUp(const EditBuffer& buf);
  void moveDown(const EditBuffer& buf);
  void moveLeft(const EditBuffer& buf);
  void moveRight(const EditBuffer& buf);
  void movePageUp(int termHeight);
  void movePageDown(const EditBuffer& buf, int termHeight);
  void moveHome();
  void moveEnd(const EditBuffer& buf);
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

enum BufOpType {
  BO_INSERT,
  BO_BACKSPACE,
  BO_DELETE,
  BO_SLIDE_UP,
  BO_SLIDE_DOWN,
};

class BufferOperation {
 public:
  BufferOperation(BufOpType ot,
                  const std::vector<BufferCursor>& cursors,
                  const std::vector<std::string>& texts);
  BufOpType opType;
  std::vector<BufferCursor> iCursors;
  std::vector<std::string> insertTexts;
  std::vector<BufferCursor> oCursors{};
  std::vector<std::string> removedTexts{};
};

class Pane {
 public:
  Pane(WINDOW* window);
  void addCursor();
  int getKeypress() const;
  void handleKeypress(int keycode);
  void loadFromFile(const std::string& filename);
  void redraw();

 private:
  int paneFocus{};
  int command{};
  int commandCursorPosition{};
  int opStackPosition{};
  WINDOW* window{};
  std::string filename{};
  std::string commandPrompt{};
  std::string userCommandArgs{};
  EditBuffer buf{};
  BufferPosition bufOffset{};
  std::vector<BufferCursor> cursors{BufferCursor{}};
  std::vector<BufferOperation> opStack{};
  SearchResults searchResults{};
  void initiateSaveCommand();
  void initiateOpenCommand();
  void initiateFindCommand();
  void saveBufferToFile(const std::string& saveTarget) const;
  void handleSearch();
  void saveBufOp(BufferOperation& bufOp);
  void undoLastBufOp();
  void redoNextBufOp();
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

  std::vector<BufferCursor> getMatches(const std::string& query) const;
  void refresh() const;
  void erase() const;
  int getGutterWidth() const;
  BufferCursor getLeadCursor() const;
};
