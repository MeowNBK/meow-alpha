#pragma once

#include "runtime/function_arity.hpp"
#include <string>
#include <functional>
#include <variant>

class Interpreter;
class Value;

using Arguments = const std::vector<Value>&;

using NativeFnSimple = std::function<Value(Arguments args)>;
using NativeFnAdvanced = std::function<Value(Interpreter* engine, Arguments args)>;

struct NativeFunction {
    std::string name;
    std::variant<NativeFnSimple, NativeFnAdvanced> function;
    Arity arity;

    NativeFunction(const std::string& n, NativeFnSimple fn, Arity a);
    NativeFunction(const std::string& n, NativeFnAdvanced fn, Arity a);
};
