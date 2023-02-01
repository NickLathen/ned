#include "pane.hh"

BufferOperation::BufferOperation(std::vector<BufferCursor>& cursors,
                                 std::string&& text,
                                 BufOpType ot)
    : iCursors{cursors}, insertText{text}, opType{ot} {}
