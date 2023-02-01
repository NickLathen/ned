#include "pane.hh"

BufferOperation::BufferOperation(BufferCursor& cursor,
                                 std::string& text,
                                 BufOpType ot)
    : targetCursor{cursor}, insertText{text}, opType{ot} {}
BufferOperation::BufferOperation(BufferCursor& cursor,
                                 std::string&& text,
                                 BufOpType ot)
    : targetCursor{cursor}, insertText{text}, opType{ot} {}
