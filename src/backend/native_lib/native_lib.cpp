#include "runtime/value.hpp"
#include "native_lib/native_lib.hpp"
#include "callable/native_callable.hpp"
#include <unordered_map>
#include <string>

struct Arity;

void NativeLibrary::registerFn(const std::string& name, NativeFnSimple fn, Arity arity) {
    auto data = std::make_shared<NativeFunction>(name, fn, arity);

    auto callable = std::make_shared<NativeCallable>(data);

    contents[name] = Value(Function(callable));
}

void NativeLibrary::registerFn(const std::string& name, NativeFnSimple fn, int arity) {
    return registerFn(name, fn, Arity::fixed(arity));
}

void NativeLibrary::registerFn(const std::string& name, NativeFnAdvanced fn, Arity arity) {
    auto data = std::make_shared<NativeFunction>(name, fn, arity);

    auto callable = std::make_shared<NativeCallable>(data);

    contents[name] = Value(Function(callable));
}

void NativeLibrary::registerFn(const std::string& name, NativeFnAdvanced fn, int arity) {
    return registerFn(name, fn, Arity::fixed(arity));
}

void NativeLibrary::registerValue(const std::string& name, const Value& value) {
    contents[name] = value;
}