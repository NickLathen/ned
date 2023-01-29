#include <ncurses.h>
#include <panel.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>
#include "const.hh"
#include "log.hh"
#include "pane.hh"

bool quitNed = false;

void signal_callback_handler(int signum) {
  endwin();
  exit(signum);
}

std::string toDirectory(std::string filepath) {
  size_t slashI = filepath.find_last_of("/");
  filepath = filepath.substr(0, slashI);
  return filepath;
}

void mainLoop(Pane& pane) {
  int keycode = pane.getKeypress();
  if (keycode == -1) {
    return;
  }
  if (keycode == CTRL_C || keycode == CTRL_Q) {
    quitNed = true;
    return;
  }
  pane.handleKeypress(keycode);
  LOGF << "key: " << keycode << "\r\n";
  LOGFLUSH;
}

int main(int argc, char** argv) {
#pragma unused(argc)
  chdir(toDirectory(argv[0]).c_str());
  logf = std::ofstream("log.txt", std::ofstream::out | std::ofstream::trunc);
  if (!logf.is_open()) {
    std::cerr << "Failed to open log.txt\n";
    return 1;
  }
  signal(SIGINT, signal_callback_handler);

  // setup ncurses
  initscr();
  start_color();
  init_pair(N_TEXT, 15, 234);
  init_pair(N_GUTTER, 247, 236);
  init_pair(N_INFO, 0, 15);
  init_pair(N_COMMAND, 15, 234);
  WINDOW* textPane = newwin(0, 0, 0, 0);
  LOGF << "LINES=" << LINES << std::endl;
  LOGF << "COLS=" << COLS << std::endl;
  keypad(textPane, true);
  intrflush(textPane, false);
  halfdelay(1);
  noecho();
  nonl();
  curs_set(0);  // hide the builtin cursor
  raw();

  Pane pane{textPane};
  pane.loadFromFile("test.txt");
  pane.addCursor();
  pane.redraw();
  while (!quitNed) {
    mainLoop(pane);
  }
  endwin();
}
