#include "runtime/value.hpp"
#include "interface/iterable.hpp"

Iterable* toIterable(const Value& value) {
    return std::visit(overloaded {
        [](const String& str) -> Iterable* {
            return str.get();
        },
        [](const Array& arr) -> Iterable* {
            return arr.get();
        },
        [](const Object& obj) -> Iterable* {
            return obj.get();
        },
        [](const Instance& ist) -> Iterable* {
            return ist.get();
        },
        [](const auto&) -> Iterable* {
            return nullptr;
        }
    }, value);
}