#include <ncurses.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <vector>
#include "log.h"
#include "pane.hh"
bool quitNed = false;

enum Key { KK_UP, KK_DOWN, KK_J, KK_K, KK_Q, KK_LEFT, KK_RIGHT, KK_COUNT };
Key getKeyPress() {
  int c = getch();
  if (c == -1)
    return KK_COUNT;
  switch (c) {
    case 'j':
      return KK_J;
    case 'k':
      return KK_K;
    case 'q':
      return KK_Q;
    case KEY_UP:
      return KK_UP;
    case KEY_DOWN:
      return KK_DOWN;
    case KEY_LEFT:
      return KK_LEFT;
    case KEY_RIGHT:
      return KK_RIGHT;
  }
  LOGF << "missing c: " << c << std::endl;
  return KK_COUNT;
}

void walkCursors(Key k, Pane& pane) {
  switch (k) {
    case KK_UP:
      pane.walkCursorsUp();
      break;
    case KK_DOWN:
      pane.walkCursorsDown();
      break;
    case KK_LEFT:
      pane.walkCursorsLeft();
      break;
    case KK_RIGHT:
      pane.walkCursorsRight();
      break;
    default:
      break;
  }
}

void mainLoop(Pane& pane) {
  Key k = getKeyPress();
  if (k != KK_COUNT) {
    if (k == KK_UP || k == KK_DOWN || k == KK_LEFT || k == KK_RIGHT) {
      erase();
      walkCursors(k, pane);
      pane.adjustOffset();
      pane.drawBuffer();
      pane.drawCursors();
    }
    LOGF << "key: " << k << "\r\n";
  }
  LOGFLUSH;
}

void signal_callback_handler(int signum) {
  endwin();
  exit(signum);
}

std::string toDirectory(std::string filepath) {
  size_t slashI = filepath.find_last_of("/");
  filepath = filepath.substr(0, slashI);
  return filepath;
}

int main(int argc, char** argv) {
  chdir(toDirectory(argv[0]).c_str());
  logf = std::ofstream("log.txt", std::ofstream::out | std::ofstream::trunc);
  if (!logf) {
    std::cerr << "Failed to open log.txt\n";
    return 1;
  }
  signal(SIGINT, signal_callback_handler);
  std::vector<std::string> lines(
      {"this is a rearendering code. this is a relly long line  lolol it will "
       "require some scrolling probalby something else when switching between "
       "lines oh lol oh lol yeah.",
       "is", "a", "test", "yo!"});
  EditBuffer buf{lines};
  // setup ncurses
  WINDOW* w = initscr();
  halfdelay(1);
  keypad(w, true);
  noecho();
  nonl();
  curs_set(0);  // hide the builtin cursor
  intrflush(w, false);
  // destroy ncurses
  Pane pane{w, buf};
  pane.setOffset(0, 2);
  pane.drawBuffer();
  pane.addCursor();
  while (!quitNed) {
    mainLoop(pane);
  }
  endwin();
}
