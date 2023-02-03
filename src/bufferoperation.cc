#include "pane.hh"

BufferOperation::BufferOperation(const std::vector<BufferCursor>& cursors,

                                 const std::vector<std::string>& texts,
                                 BufOpType ot)
    : opType{ot}, iCursors{cursors}, insertTexts{texts} {}
