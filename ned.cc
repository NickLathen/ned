#include "log.hh"
#include "pane.hh"
#include "testdata.hh"
#include <fstream>
#include <iostream>
#include <ncurses.h>
#include <signal.h>
#include <sstream>
#include <time.h>
#include <unistd.h>
#include <vector>

bool quitNed = false;

void walkCursors(int k, Pane &pane) {
  switch (k) {
  case KEY_UP:
    pane.walkCursorsUp();
    break;
  case KEY_DOWN:
    pane.walkCursorsDown();
    break;
  case KEY_LEFT:
    pane.walkCursorsLeft();
    break;
  case KEY_RIGHT:
    pane.walkCursorsRight();
    break;
  default:
    break;
  }
}

void mainLoop(Pane &pane) {
  int k = getch();
  if (k == 3 /*ctrl-c*/ || k == 17 /*ctrl-q*/) {
    quitNed = true;
    return;
  }
  if (k == KEY_UP || k == KEY_DOWN || k == KEY_LEFT || k == KEY_RIGHT) {
    erase();
    walkCursors(k, pane);
    pane.adjustOffset();
    pane.drawBuffer();
    pane.drawCursors();
  }
  if (k != -1) {
    LOGF << "key: " << k << "\r\n";
    LOGFLUSH;
  }
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

std::vector<std::string> toLines(std::string s) {
  std::vector<std::string> result;
  int start = 0;
  for (int i = 0; i < (int)s.size(); i++) {
    if (s[i] == '\n' || i == (int)s.size() - 1) {
      result.push_back(s.substr(start, i - start));
      start = i + 1;
    }
  }
  return result;
};

EditBuffer loadFile(std::string file) {
  std::ifstream ifile{file};
  std::vector<std::string> lines;
  std::string line;
  while (std::getline(ifile, line)) {
    lines.push_back(line);
    line = "";
  }
  EditBuffer buf{lines};
  return buf;
}

int main(int argc, char **argv) {
#pragma unused(argc)
  chdir(toDirectory(argv[0]).c_str());
  logf = std::ofstream("log.txt", std::ofstream::out | std::ofstream::trunc);
  if (!logf) {
    std::cerr << "Failed to open log.txt\n";
    return 1;
  }
  signal(SIGINT, signal_callback_handler);
  EditBuffer buf = loadFile("test.txt");
  // setup ncurses
  WINDOW *w = initscr();
  halfdelay(1);
  keypad(w, true);
  noecho();
  nonl();
  curs_set(0); // hide the builtin cursor
  intrflush(w, false);
  raw();
  Pane pane{w, buf};
  // pane.setOffset(2, 0);
  pane.drawBuffer();
  pane.addCursor();
  while (!quitNed) {
    mainLoop(pane);
  }
  endwin();
}
