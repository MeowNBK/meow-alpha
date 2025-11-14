#include "diagnostics/console_output.hpp"
#include <iostream>
#ifdef _WIN32
    #include <windows.h>
#endif

void setConsoleOutput() {
    std::ios::sync_with_stdio(false);
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}
