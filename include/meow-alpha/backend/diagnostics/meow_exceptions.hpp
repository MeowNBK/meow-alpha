#pragma once

#include "runtime/value.hpp"
#include <string>
#include <stdexcept>

struct ReturnException: public std::runtime_error {
    Value value;
    ReturnException(Value value): std::runtime_error("Return"), value(value) {}
};

struct BreakException: public std::runtime_error {
    BreakException(): std::runtime_error("Break") {}
};

struct ContinueException: public std::runtime_error {
    ContinueException(): std::runtime_error("Continue") {}
};

struct FunctionException: public std::runtime_error {
    FunctionException(const std::string &msg): std::runtime_error(msg) {}
};

struct MeowScriptException: public std::runtime_error {
    Value value;

    MeowScriptException(Value val): std::runtime_error("An exception was thrown in MeowScript"), value(std::move(val)) {}
};