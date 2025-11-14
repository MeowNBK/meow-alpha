#include "runtime/value.hpp"
#include "interface/stringifiable.hpp"

Stringifiable* toStringifiable(const Value& value) {
    return std::visit(overloaded {
        [](const String& str) -> Stringifiable* {
            return str.get();
        },
        [](const Array& arr) -> Stringifiable* {
            return arr.get();
        },
        [](const Object& obj) -> Stringifiable* {
            return obj.get();
        },
        [](const Class& obj) -> Stringifiable* {
            return obj.get();
        },
        [](const Instance& obj) -> Stringifiable* {
            return obj.get();
        },
        [](const BoundMethod& obj) -> Stringifiable* {
            return obj.get();
        },
        [](const auto&) -> Stringifiable* {
            return nullptr;
        }
    }, value);
}