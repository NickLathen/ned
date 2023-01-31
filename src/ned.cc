#include <ncurses.h>
#include <signal.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include "const.hh"
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
  std::cout << "key: " << keycode << std::endl;
  if (keycode == -1) {
    return;
  }
  if (keycode == CTRL_C || keycode == CTRL_Q) {
    quitNed = true;
    return;
  }
  pane.handleKeypress(keycode);
}

int main(int argc, char** argv) {
  std::streambuf* coutBackup;
  chdir(toDirectory(argv[0]).c_str());

  coutBackup = std::cout.rdbuf();
  std::ofstream logfile("log.txt", std::ios_base::out | std::ios_base::trunc);
  std::cout.rdbuf(logfile.rdbuf());

  signal(SIGINT, signal_callback_handler);

  // setup ncurses
  initscr();
  start_color();
  init_pair(N_TEXT, 15, 234);
  init_pair(N_GUTTER, 247, 236);
  init_pair(N_INFO, 0, 15);
  init_pair(N_COMMAND, 15, 234);
  init_pair(N_HIGHLIGHT, 11, 21);
  WINDOW* textPane = newwin(0, 0, 0, 0);
  std::cout << "LINES=" << LINES << std::endl;
  std::cout << "COLS=" << COLS << std::endl;
  keypad(textPane, true);
  intrflush(textPane, false);
  halfdelay(1);
  noecho();
  nonl();
  curs_set(0);
  raw();

  // prepare pane
  Pane pane{textPane};
  if (argc == 2) {
    pane.loadFromFile(argv[1]);
  }
  pane.redraw();

  // MAIN LOOP
  while (!quitNed) {
    mainLoop(pane);
  }

  // clean up
  std::cout.rdbuf(coutBackup);
  logfile.close();
  endwin();
}
