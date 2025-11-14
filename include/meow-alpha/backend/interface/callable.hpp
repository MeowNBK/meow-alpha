#pragma once

#include "runtime/function_arity.hpp"
#include <vector>
#include <memory>

class Interpreter;
class Value;
class Environment;
struct Arity;

class Callable {
public:

    virtual ~Callable() = default;

    virtual Value call(Interpreter* engine, const std::vector<Value>& args) = 0;

    virtual Arity arity() const = 0;

    virtual std::shared_ptr<Environment> getEnv() {
        return nullptr;
    }
};

using Function = std::shared_ptr<Callable>;