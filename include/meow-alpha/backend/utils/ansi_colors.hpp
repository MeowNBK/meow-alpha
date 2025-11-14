#pragma once
#include <string_view>

namespace AnsiColors {
    constexpr std::string_view RESET          = "\x1b[0m";
    constexpr std::string_view BOLD           = "\x1b[1m";
    constexpr std::string_view RED            = "\x1b[31m";
    constexpr std::string_view GREEN          = "\x1b[32m";
    constexpr std::string_view YELLOW         = "\x1b[33m";
    constexpr std::string_view BLUE           = "\x1b[34m";
    constexpr std::string_view MAGENTA        = "\x1b[35m";
    constexpr std::string_view CYAN           = "\x1b[36m";
    constexpr std::string_view BRIGHT_RED     = "\x1b[91m";
    constexpr std::string_view BRIGHT_GREEN   = "\x1b[92m";
    constexpr std::string_view BRIGHT_YELLOW  = "\x1b[93m";
    constexpr std::string_view BRIGHT_BLUE    = "\x1b[94m";
    constexpr std::string_view BRIGHT_MAGENTA = "\x1b[95m";
    constexpr std::string_view BRIGHT_CYAN    = "\x1b[96m";
}
