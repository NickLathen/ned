#include "pane.hh"

BufferOperation::BufferOperation(const std::vector<BufferCursor>& cursors,
                                 const std::string&& text,
                                 BufOpType ot)
    : iCursors{cursors}, insertText{text}, opType{ot} {}
