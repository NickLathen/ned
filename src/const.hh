#pragma once
#define CTRL_C 3
#define CTRL_O 15
#define CTRL_Q 17
#define CTRL_S 19
#define TAB 9
#define CARRIAGE_RETURN 13
#define BACKSPACE 263
#define DELETE 330

void signal_callback_handler(int signum);
