#include "native_lib/standard_lib.hpp"
#include "diagnostics/meow_exceptions.hpp"
#include "runtime/interpreter.hpp"
#include <cstdlib>

Value native_exit(Arguments args) {
    Int exit_code = 0;
    if (!args.empty()) {
        if (std::holds_alternative<Int>(args[0])) {
            exit_code = std::get<Int>(args[0]);
        } else {
             throw FunctionException("Mã thoát của hàm exit() phải là một số nguyên.");
        }
    }
    std::exit(exit_code);
    return Value(Null{});
}

Value native_get_env(Arguments args) {
    if (args.empty() || !std::holds_alternative<String>(args[0])) {
        throw FunctionException("Hàm getEnv() cần 1 tham số là tên biến (chuỗi).");
    }
    
    const char* varName = std::get<String>(args[0])->str.c_str();
    const char* value = std::getenv(varName);
    
    if (value == nullptr) {
        return Value(Null{});
    }
    
    return Value(std::string(value));
}

Value native_system_exec(Arguments args) {
    const auto& command = std::get<String>(args[0])->str;

    int exit_code = std::system(command.c_str());
    return Value((Int)exit_code);
}

Value nativeArgv(Interpreter* engine, Arguments args) {
    Array arr = std::make_shared<ArrayData>();
    arr->elements.reserve(engine->getArgv().size());

    for (const auto &a : engine->getArgv()) {
        String str = std::make_shared<StringData>(a);
        arr->elements.push_back(Value(str));
    }

    return Value(arr);
}

SystemLib::SystemLib() {
    registerFn("exit", native_exit, Arity::range(0, 1));
    registerFn("getEnv", native_get_env, 1);
    registerFn("exec", native_system_exec, 1);
    registerFn("argv", nativeArgv, 0);
}