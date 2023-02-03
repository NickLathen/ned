#include "pane.hh"

BufferOperation::BufferOperation(BufOpType ot,
                                 const std::vector<BufferCursor>& cursors,
                                 const std::vector<std::string>& texts)
    : opType{ot}, iCursors{cursors}, insertTexts{texts} {}
