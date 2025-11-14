#include "interface/callable.hpp"
#include "callable/native_callable.hpp"
#include "runtime/value.hpp"
#include "runtime/native_fn.hpp"
#include "runtime/overload.hpp"

NativeCallable::NativeCallable(std::shared_ptr<NativeFunction> data): functionData(std::move(data)) {}

Value NativeCallable::call(Interpreter* engine, const std::vector<Value>& args) {
    return std::visit(overloaded {
        [&](const NativeFnSimple& fn) {
            return fn(args);
        },
        [&](const NativeFnAdvanced& fn) {
            return fn(engine, args);
        }
    }, functionData->function);
}

Arity NativeCallable::arity() const {
    return functionData->arity;
}