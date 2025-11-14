#include "common/source_file.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "diagnostics/console_output.hpp"
#include "module/module_manager.hpp"
#include <memory>
#include <iostream>

int main(int argc, char* argv[]) {
    setConsoleOutput();

    if (argc < 2) {
        std::cerr << "Cần ít nhất 2 tham số!";
        return 1;
    }
    ModuleManager manager(argc, argv);
    
    try {
        manager.load("", argv[1]);
    } catch (const Diagnostic& e) {
        std::cerr << e.str() << "\n";
    } catch (const std::runtime_error& e) {
        std::cerr << "Lỗi runtime không xác định: " << e.what() << "\n";
    }
}