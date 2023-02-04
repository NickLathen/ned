#include <ncurses.h>
#include <signal.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include "const.hh"
#include "pane.hh"
//hello world

bool quitNed = false;
std::streambuf* coutBackup = nullptr;
std::ofstream* logfilep = nullptr;

void exitNed(int signum) {
  std::cout.rdbuf(coutBackup);
  logfilep->close();
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

#define RGB_TUPLE(HEX) (HEX >> 16) & (0xFF), (HEX >> 8) & (0xFF), (HEX & 0xFF)
void setupColors() {
  start_color();
  enum CCOLORS {
    seashell = 100,
    pewter,
    gray,
    ivory,
    lightwhite,
    midgray,
    darkgray,
    lightgray,
  };
  init_color(seashell, RGB_TUPLE(0xE7D2CC));
  init_color(pewter, RGB_TUPLE(0xB9B7BD));
  init_color(gray, RGB_TUPLE(0x868B8E));
  init_color(ivory, RGB_TUPLE(0xF4EAE6));
  init_color(lightwhite, RGB_TUPLE(0xF3F5F9));
  init_color(midgray, RGB_TUPLE(0x746C70));
  init_color(darkgray, RGB_TUPLE(0x4E4F50));
  init_color(lightgray, RGB_TUPLE(0xEAEFF2));
  init_pair(N_TEXT, 15, darkgray);
  init_pair(N_GUTTER, lightwhite, gray);
  init_pair(N_INFO, darkgray, lightwhite);
  init_pair(N_COMMAND, 15, darkgray);
  init_pair(N_HIGHLIGHT, 15, 39);
}
#undef RGB_TUPLE

int main(int argc, char** argv) {
  chdir(toDirectory(argv[0]).c_str());

  coutBackup = std::cout.rdbuf();
  std::ofstream logfile("log.txt", std::ios_base::out | std::ios_base::trunc);
  logfilep = &logfile;
  std::cout.rdbuf(logfile.rdbuf());

  signal(SIGINT, exitNed);

  // setup ncurses
  initscr();
  setupColors();
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

  Pane pane{textPane};
  if (argc == 2) {
    pane.loadFromFile(argv[1]);
  }
  pane.redraw();

  // MAIN LOOP
  while (!quitNed) {
    mainLoop(pane);
  }

  exitNed(0);
}
