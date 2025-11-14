#include "runtime/value.hpp"
#include "interface/indexable.hpp"

Indexable* toIndexable(const Value& value) {
    return std::visit(overloaded {
        [](const String& str) -> Indexable* {
            return str.get();
        },
        [](const Array& arr) -> Indexable* {
            return arr.get();
        },
        [](const Object& obj) -> Indexable* {
            return obj.get();
        },
        [](const Class& cls) -> Indexable* {
            return cls.get();
        },
        [](const Instance& inst) -> Indexable* {
            return inst.get();
        },
        [](const auto&) -> Indexable* {
            return nullptr;
        }
    }, value);
}