#pragma once
#include <fstream>

inline std::ofstream logf;
#define LOGF logf
#define LOGFLUSH logf.flush();