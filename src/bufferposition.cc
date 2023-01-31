#include "pane.hh"

bool operator==(const BufferPosition& a, const BufferPosition& b) {
  return a.row == b.row && a.col == b.col;
}
bool operator!=(const BufferPosition& a, const BufferPosition& b) {
  return !(a == b);
}
bool operator<(const BufferPosition& a, const BufferPosition& b) {
  return a.row < b.row || (a.row == b.row && a.col < b.col);
}
bool operator<=(const BufferPosition& a, const BufferPosition& b) {
  return (a < b) || (a == b);
}
bool operator>(const BufferPosition& a, const BufferPosition& b) {
  return a.row > b.row || (a.row == b.row && a.col > b.col);
}
bool operator>=(const BufferPosition& a, const BufferPosition& b) {
  return (a > b) || (a == b);
}
