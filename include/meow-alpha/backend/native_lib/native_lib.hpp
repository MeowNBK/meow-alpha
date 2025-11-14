#pragma once

#include "runtime/value.hpp"
#include "runtime/native_fn.hpp"
#include "callable/native_callable.hpp"
#include <unordered_map>
#include <string>

struct Arity;

class NativeLibrary {
public:
    std::unordered_map<std::string, Value> contents;

    void registerFn(const std::string& name, NativeFnSimple fn, Arity arity);

    void registerFn(const std::string& name, NativeFnSimple fn, int arity);

    void registerFn(const std::string& name, NativeFnAdvanced fn, Arity arity);

    void registerFn(const std::string& name, NativeFnAdvanced fn, int arity);
    
    void registerValue(const std::string& name, const Value& value);
};