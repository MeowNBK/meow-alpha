#pragma once

#include "runtime/value.hpp"
#include "common/token.hpp"
#include "runtime/overload.hpp"
#include <string>
#include <variant>

enum class ValueType {
    Null, Int, Real, Bool, String, Array, Object, Function, Instance, Class, BoundMethod, Total
};

inline ValueType get_value_type(const Value& value) noexcept {
    return std::visit(overloaded{
        [](Null) { return ValueType::Null; },
        [](Int) { return ValueType::Int; },
        [](Real) { return ValueType::Real; },
        [](Bool) { return ValueType::Bool; },
        [](const String&) { return ValueType::String; },
        [](const Array&) { return ValueType::Array; },
        [](const Object&) { return ValueType::Object; },
        [](const Function&) { return ValueType::Function; },
        [](const Instance&) { return ValueType::Instance; },
        [](const Class&) { return ValueType::Class; },
        [](const BoundMethod&) { return ValueType::BoundMethod; }
    }, static_cast<const BaseValue&>(value));
}

using BinaryOpFunction = Value (*)(const Value&, const Value&);
using UnaryOpFunction = Value (*)(const Value&);

constexpr size_t NUM_TOKEN_TYPES = static_cast<size_t>(TokenType::_TOTAL_TOKENS);
constexpr size_t NUM_VALUE_TYPES = static_cast<size_t>(ValueType::Total);

[[nodiscard]] inline constexpr size_t operator+(TokenType op_code) noexcept {
    return static_cast<size_t>(op_code);
}
[[nodiscard]] inline constexpr size_t operator+(ValueType value_type) noexcept {
    return static_cast<size_t>(value_type);
}

class OperatorDispatcher {
private:
    BinaryOpFunction binary_dispatch_table_[NUM_TOKEN_TYPES][NUM_VALUE_TYPES][NUM_VALUE_TYPES];
    UnaryOpFunction unary_dispatch_table_[NUM_TOKEN_TYPES][NUM_VALUE_TYPES];

public:
    OperatorDispatcher() noexcept;
    [[nodiscard]] inline const BinaryOpFunction* find(TokenType op, const Value& left, const Value& right) const noexcept {
        auto left_type = get_value_type(left);
        auto right_type = get_value_type(right);
        
        const BinaryOpFunction* function =
            &binary_dispatch_table_[+op][+left_type][+right_type];
        return function;
    }

    [[nodiscard]] inline const UnaryOpFunction* find(TokenType op, const Value& right) const noexcept {
        auto right_type = get_value_type(right);

        const UnaryOpFunction* function = &unary_dispatch_table_[+op][+right_type];
        return function;
    }
};