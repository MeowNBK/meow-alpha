#pragma once

#include "interface/callable.hpp"
#include "runtime/value.hpp"
#include "runtime/native_fn.hpp"
#include "runtime/overload.hpp"

class NativeCallable: public Callable {
public:
    std::shared_ptr<NativeFunction> functionData;

    NativeCallable(std::shared_ptr<NativeFunction> data);

    Value call(Interpreter* engine, const std::vector<Value>& args) override;

    Arity arity() const override;
};