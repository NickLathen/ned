#include "pane.hh"

BufferOperation::BufferOperation(BufferCursor& cursor,
                                 std::vector<BufferCursor>& cursors,
                                 std::string& text)
    : iCursors{cursors}, targetCursor{cursor}, insertText{text} {}
