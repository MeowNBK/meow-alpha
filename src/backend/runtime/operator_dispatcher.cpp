#include "runtime/operator_dispatcher.hpp"
#include <stdexcept>
#include <cmath>
#include <limits>
namespace {
    Int int_from_char(const String& s) {
        if (s->str.length() != 1) {
            throw std::runtime_error("Cannot convert multi-character string to Int.");
        }
        return static_cast<Int>(s->str[0]);
    }

    Int bool_to_int(const Bool& b) {
        return static_cast<Int>(b);
    }

    Real bool_to_real(const Bool& b) {
        return static_cast<Real>(b);
    }

    Value multiply_array_by_int(const Value& l_array, const Value& r_int) {
        const auto& original_elements = std::get<Array>(l_array)->elements;
        Int multiplier = std::get<Int>(r_int);
        Array new_array = std::make_shared<ArrayData>();
        if (multiplier <= 0) return Value(new_array);
        
        new_array->elements.reserve(original_elements.size() * multiplier);
        for (Int i = 0; i < multiplier; ++i) {
            new_array->elements.insert(new_array->elements.end(), original_elements.begin(), original_elements.end());
        }
        return Value(new_array);
    }
}


#define BINARY(token, left, right) binary_dispatch_table_[+token][+left][+right] = [](const Value& l, const Value& r) -> Value
#define UNARY(token, right) unary_dispatch_table_[+token][+right] = [](const Value& r) -> Value

OperatorDispatcher::OperatorDispatcher() noexcept {
    for (size_t i = 0; i < NUM_TOKEN_TYPES; ++i) {
        for (size_t j = 0; j < NUM_VALUE_TYPES; ++j) {
            unary_dispatch_table_[i][j] = nullptr;
            for (size_t k = 0; k < NUM_VALUE_TYPES; ++k) {
                binary_dispatch_table_[i][j][k] = nullptr;
            }
        }
    }

    using enum TokenType;
    using VT = ValueType;

    // OP_PLUS
    BINARY(OP_PLUS, VT::Int, VT::Int) { return std::get<Int>(l) + std::get<Int>(r); };
    BINARY(OP_PLUS, VT::Real, VT::Real) { return std::get<Real>(l) + std::get<Real>(r); };
    BINARY(OP_PLUS, VT::Int, VT::Real) { return static_cast<Real>(std::get<Int>(l)) + std::get<Real>(r); };
    BINARY(OP_PLUS, VT::Real, VT::Int) { return std::get<Real>(l) + static_cast<Real>(std::get<Int>(r)); };
    BINARY(OP_PLUS, VT::String, VT::String) { return Value(std::get<String>(l)->str + std::get<String>(r)->str); };
    BINARY(OP_PLUS, VT::String, VT::Int) { return Value(std::get<String>(l)->str + std::to_string(std::get<Int>(r))); };
    BINARY(OP_PLUS, VT::Int, VT::String) { return Value(std::to_string(std::get<Int>(l)) + std::get<String>(r)->str); };
    BINARY(OP_PLUS, VT::String, VT::Real) { return Value(std::get<String>(l)->str + std::to_string(std::get<Real>(r))); };
    BINARY(OP_PLUS, VT::Real, VT::String) { return Value(std::to_string(std::get<Real>(l)) + std::get<String>(r)->str); };
    BINARY(OP_PLUS, VT::Int, VT::Bool) { return std::get<Int>(l) + bool_to_int(std::get<Bool>(r)); };
    BINARY(OP_PLUS, VT::Bool, VT::Int) { return bool_to_int(std::get<Bool>(l)) + std::get<Int>(r); };
    BINARY(OP_PLUS, VT::Real, VT::Bool) { return std::get<Real>(l) + bool_to_real(std::get<Bool>(r)); };
    BINARY(OP_PLUS, VT::Bool, VT::Real) { return bool_to_real(std::get<Bool>(l)) + std::get<Real>(r); };
    BINARY(OP_PLUS, VT::String, VT::Null) { return Value(std::get<String>(l)->str + toString(r)); };
    BINARY(OP_PLUS, VT::Null, VT::String) { return Value(toString(l) + std::get<String>(r)->str); };
    BINARY(OP_PLUS, VT::String, VT::Array) { return Value(std::get<String>(l)->str + toString(r)); };
    BINARY(OP_PLUS, VT::Array, VT::String) { return Value(toString(l) + std::get<String>(r)->str); };
    BINARY(OP_PLUS, VT::String, VT::Object) { return Value(std::get<String>(l)->str + toString(r)); };
    BINARY(OP_PLUS, VT::Object, VT::String) { return Value(toString(l) + std::get<String>(r)->str); };
    BINARY(OP_PLUS, VT::Array, VT::Array) {
        Array new_arr = std::make_shared<ArrayData>();
        new_arr->elements = std::get<Array>(l)->elements;
        const auto& r_elements = std::get<Array>(r)->elements;
        new_arr->elements.insert(new_arr->elements.end(), r_elements.begin(), r_elements.end());
        return Value(new_arr);
    };

    // OP_MINUS
    BINARY(OP_MINUS, VT::Int, VT::Int) { return std::get<Int>(l) - std::get<Int>(r); };
    BINARY(OP_MINUS, VT::Real, VT::Real) { return std::get<Real>(l) - std::get<Real>(r); };
    BINARY(OP_MINUS, VT::Int, VT::Real) { return static_cast<Real>(std::get<Int>(l)) - std::get<Real>(r); };
    BINARY(OP_MINUS, VT::Real, VT::Int) { return std::get<Real>(l) - static_cast<Real>(std::get<Int>(r)); };
    BINARY(OP_MINUS, VT::Int, VT::Bool) { return std::get<Int>(l) - bool_to_int(std::get<Bool>(r)); };
    BINARY(OP_MINUS, VT::Bool, VT::Int) { return bool_to_int(std::get<Bool>(l)) - std::get<Int>(r); };
    BINARY(OP_MINUS, VT::Real, VT::Bool) { return std::get<Real>(l) - bool_to_real(std::get<Bool>(r)); };
    BINARY(OP_MINUS, VT::Bool, VT::Real) { return bool_to_real(std::get<Bool>(l)) - std::get<Real>(r); };

    // OP_MULTIPLY
    BINARY(OP_MULTIPLY, VT::Int, VT::Int) { return std::get<Int>(l) * std::get<Int>(r); };
    BINARY(OP_MULTIPLY, VT::Real, VT::Real) { return std::get<Real>(l) * std::get<Real>(r); };
    BINARY(OP_MULTIPLY, VT::Int, VT::Real) { return static_cast<Real>(std::get<Int>(l)) * std::get<Real>(r); };
    BINARY(OP_MULTIPLY, VT::Real, VT::Int) { return std::get<Real>(l) * static_cast<Real>(std::get<Int>(r)); };
    BINARY(OP_MULTIPLY, VT::String, VT::Int) {
        std::string res;
        for (Int i = 0; i < std::get<Int>(r); ++i) res += std::get<String>(l)->str;
        return Value(res);
    };
    BINARY(OP_MULTIPLY, VT::Int, VT::String) {
        std::string res;
        for (Int i = 0; i < std::get<Int>(l); ++i) res += std::get<String>(r)->str;
        return Value(res);
    };
    BINARY(OP_MULTIPLY, VT::Int, VT::Bool) { return std::get<Int>(l) * bool_to_int(std::get<Bool>(r)); };
    BINARY(OP_MULTIPLY, VT::Bool, VT::Int) { return bool_to_int(std::get<Bool>(l)) * std::get<Int>(r); };
    BINARY(OP_MULTIPLY, VT::Real, VT::Bool) { return std::get<Real>(l) * bool_to_real(std::get<Bool>(r)); };
    BINARY(OP_MULTIPLY, VT::Bool, VT::Real) { return bool_to_real(std::get<Bool>(l)) * std::get<Real>(r); };
    BINARY(OP_MULTIPLY, VT::Array, VT::Int) { return multiply_array_by_int(l, r); };
    BINARY(OP_MULTIPLY, VT::Int, VT::Array) { return multiply_array_by_int(r, l); };

    // OP_DIVIDE
    BINARY(OP_DIVIDE, VT::Int, VT::Int) {
        if (std::get<Int>(r) == 0) {
            if (std::get<Int>(l) == 0) return std::numeric_limits<Real>::quiet_NaN();
            return (std::get<Int>(l) > 0) ? std::numeric_limits<Real>::infinity() : -std::numeric_limits<Real>::infinity();
        }
        return static_cast<Real>(std::get<Int>(l)) / static_cast<Real>(std::get<Int>(r));
    };
    BINARY(OP_DIVIDE, VT::Real, VT::Real) { return std::get<Real>(l) / std::get<Real>(r); };
    BINARY(OP_DIVIDE, VT::Int, VT::Real) {
        if (std::get<Real>(r) == 0.0) {
            if (std::get<Int>(l) == 0) return std::numeric_limits<Real>::quiet_NaN();
            return (std::get<Int>(l) > 0) ? std::numeric_limits<Real>::infinity() : -std::numeric_limits<Real>::infinity();
        }
        return static_cast<Real>(std::get<Int>(l)) / std::get<Real>(r);
    };
    BINARY(OP_DIVIDE, VT::Real, VT::Int) {
        if (std::get<Int>(r) == 0) {
            if (std::get<Real>(l) == 0.0) return std::numeric_limits<Real>::quiet_NaN();
            return (std::get<Real>(l) > 0.0) ? std::numeric_limits<Real>::infinity() : -std::numeric_limits<Real>::infinity();
        }
        return std::get<Real>(l) / static_cast<Real>(std::get<Int>(r));
    };
    BINARY(OP_DIVIDE, VT::Int, VT::Bool) {
        Int divisor = bool_to_int(std::get<Bool>(r));
        if (divisor == 0) {
            if (std::get<Int>(l) == 0) return std::numeric_limits<Real>::quiet_NaN();
            return (std::get<Int>(l) > 0) ? std::numeric_limits<Real>::infinity() : -std::numeric_limits<Real>::infinity();
        }
        return static_cast<Real>(std::get<Int>(l)) / static_cast<Real>(divisor);
    };
    BINARY(OP_DIVIDE, VT::Real, VT::Bool) {
        Real divisor = bool_to_real(std::get<Bool>(r));
        if (divisor == 0.0) {
            if (std::get<Real>(l) == 0.0) return std::numeric_limits<Real>::quiet_NaN();
            return (std::get<Real>(l) > 0.0) ? std::numeric_limits<Real>::infinity() : -std::numeric_limits<Real>::infinity();
        }
        return std::get<Real>(l) / divisor;
    };

    // OP_MODULO
    BINARY(OP_MODULO, VT::Int, VT::Int) {
        if (std::get<Int>(r) == 0) throw std::runtime_error("Modulo cho 0.");
        return std::get<Int>(l) % std::get<Int>(r);
    };
    BINARY(OP_MODULO, VT::Int, VT::Bool) {
        Int divisor = bool_to_int(std::get<Bool>(r));
        if (divisor == 0) throw std::runtime_error("Modulo cho 0.");
        return std::get<Int>(l) % divisor;
    };

    // OP_EXPONENT
    BINARY(OP_EXPONENT, VT::Int, VT::Int) { return std::pow(static_cast<Real>(std::get<Int>(l)), static_cast<Real>(std::get<Int>(r))); };
    BINARY(OP_EXPONENT, VT::Real, VT::Real) { return std::pow(std::get<Real>(l), std::get<Real>(r)); };
    BINARY(OP_EXPONENT, VT::Int, VT::Real) { return std::pow(static_cast<Real>(std::get<Int>(l)), std::get<Real>(r)); };
    BINARY(OP_EXPONENT, VT::Real, VT::Int) { return std::pow(std::get<Real>(l), static_cast<Real>(std::get<Int>(r))); };

    // OP_EQ
    BINARY(OP_EQ, VT::Int, VT::Int) { return std::get<Int>(l) == std::get<Int>(r); };
    BINARY(OP_EQ, VT::Real, VT::Real) { return std::get<Real>(l) == std::get<Real>(r); };
    BINARY(OP_EQ, VT::Int, VT::Real) { return static_cast<Real>(std::get<Int>(l)) == std::get<Real>(r); };
    BINARY(OP_EQ, VT::Real, VT::Int) { return std::get<Real>(l) == static_cast<Real>(std::get<Int>(r)); };
    BINARY(OP_EQ, VT::Bool, VT::Bool) { return std::get<Bool>(l) == std::get<Bool>(r); };
    BINARY(OP_EQ, VT::String, VT::String) { return std::get<String>(l)->str == std::get<String>(r)->str; };
    BINARY(OP_EQ, VT::Null, VT::Null) { return true; };
    BINARY(OP_EQ, VT::Bool, VT::Int) { return bool_to_int(std::get<Bool>(l)) == std::get<Int>(r); };
    BINARY(OP_EQ, VT::Int, VT::Bool) { return std::get<Int>(l) == bool_to_int(std::get<Bool>(r)); };
    BINARY(OP_EQ, VT::Null, VT::String) { return std::get<String>(r)->str.empty(); };
    BINARY(OP_EQ, VT::String, VT::Null) { return std::get<String>(l)->str.empty(); };
    BINARY(OP_EQ, VT::Null, VT::Bool) { return std::get<Bool>(r) == false; };
    BINARY(OP_EQ, VT::Bool, VT::Null) { return std::get<Bool>(l) == false; };
    BINARY(OP_EQ, VT::Null, VT::Int) { return false; };
    BINARY(OP_EQ, VT::Int, VT::Null) { return false; };
    BINARY(OP_EQ, VT::Null, VT::Real) { return false; };
    BINARY(OP_EQ, VT::Real, VT::Null) { return false; };
    BINARY(OP_EQ, VT::Null, VT::Array) { return std::get<Array>(r)->elements.empty(); };
    BINARY(OP_EQ, VT::Array, VT::Null) { return std::get<Array>(l)->elements.empty(); };
    BINARY(OP_EQ, VT::Null, VT::Object) { return std::get<Object>(r)->pairs.empty(); };
    BINARY(OP_EQ, VT::Object, VT::Null) { return std::get<Object>(l)->pairs.empty(); };
    BINARY(OP_EQ, VT::Null, VT::Function) { return false; };
    BINARY(OP_EQ, VT::Function, VT::Null) { return false; };
    BINARY(OP_EQ, VT::Null, VT::Instance) { return false; };
    BINARY(OP_EQ, VT::Instance, VT::Null) { return false; };
    BINARY(OP_EQ, VT::Null, VT::Class) { return false; };
    BINARY(OP_EQ, VT::Class, VT::Null) { return false; };
    BINARY(OP_EQ, VT::Null, VT::BoundMethod) { return false; };
    BINARY(OP_EQ, VT::BoundMethod, VT::Null) { return false; };

    // OP_NEQ
    BINARY(OP_NEQ, VT::Int, VT::Int) { return std::get<Int>(l) != std::get<Int>(r); };
    BINARY(OP_NEQ, VT::Real, VT::Real) { return std::get<Real>(l) != std::get<Real>(r); };
    BINARY(OP_NEQ, VT::Int, VT::Real) { return static_cast<Real>(std::get<Int>(l)) != std::get<Real>(r); };
    BINARY(OP_NEQ, VT::Real, VT::Int) { return std::get<Real>(l) != static_cast<Real>(std::get<Int>(r)); };
    BINARY(OP_NEQ, VT::Bool, VT::Bool) { return std::get<Bool>(l) != std::get<Bool>(r); };
    BINARY(OP_NEQ, VT::String, VT::String) { return std::get<String>(l)->str != std::get<String>(r)->str; };
    BINARY(OP_NEQ, VT::Null, VT::Null) { return false; };
    BINARY(OP_NEQ, VT::Bool, VT::Int) { return bool_to_int(std::get<Bool>(l)) != std::get<Int>(r); };
    BINARY(OP_NEQ, VT::Int, VT::Bool) { return std::get<Int>(l) != bool_to_int(std::get<Bool>(r)); };
    BINARY(OP_NEQ, VT::Null, VT::String) { return !std::get<String>(r)->str.empty(); };
    BINARY(OP_NEQ, VT::String, VT::Null) { return !std::get<String>(l)->str.empty(); };
    BINARY(OP_NEQ, VT::Null, VT::Bool) { return std::get<Bool>(r) == true; };
    BINARY(OP_NEQ, VT::Bool, VT::Null) { return std::get<Bool>(l) == true; };
    BINARY(OP_NEQ, VT::Null, VT::Int) { return true; };
    BINARY(OP_NEQ, VT::Int, VT::Null) { return true; };
    BINARY(OP_NEQ, VT::Null, VT::Real) { return true; };
    BINARY(OP_NEQ, VT::Real, VT::Null) { return true; };
    BINARY(OP_NEQ, VT::Null, VT::Array) { return !std::get<Array>(r)->elements.empty(); };
    BINARY(OP_NEQ, VT::Array, VT::Null) { return !std::get<Array>(l)->elements.empty(); };
    BINARY(OP_NEQ, VT::Null, VT::Object) { return !std::get<Object>(r)->pairs.empty(); };
    BINARY(OP_NEQ, VT::Object, VT::Null) { return !std::get<Object>(l)->pairs.empty(); };
    BINARY(OP_NEQ, VT::Null, VT::Function) { return true; };
    BINARY(OP_NEQ, VT::Function, VT::Null) { return true; };
    BINARY(OP_NEQ, VT::Null, VT::Instance) { return true; };
    BINARY(OP_NEQ, VT::Instance, VT::Null) { return true; };
    BINARY(OP_NEQ, VT::Null, VT::Class) { return true; };
    BINARY(OP_NEQ, VT::Class, VT::Null) { return true; };
    BINARY(OP_NEQ, VT::Null, VT::BoundMethod) { return true; };
    BINARY(OP_NEQ, VT::BoundMethod, VT::Null) { return true; };

    // OP_LT
    BINARY(OP_LT, VT::Int, VT::Int) { return std::get<Int>(l) < std::get<Int>(r); };
    BINARY(OP_LT, VT::Real, VT::Real) { return std::get<Real>(l) < std::get<Real>(r); };
    BINARY(OP_LT, VT::Int, VT::Real) { return static_cast<Real>(std::get<Int>(l)) < std::get<Real>(r); };
    BINARY(OP_LT, VT::Real, VT::Int) { return std::get<Real>(l) < static_cast<Real>(std::get<Int>(r)); };
    BINARY(OP_LT, VT::String, VT::String) { return std::get<String>(l)->str < std::get<String>(r)->str; };
    BINARY(OP_LT, VT::Bool, VT::Int) { return bool_to_int(std::get<Bool>(l)) < std::get<Int>(r); };
    BINARY(OP_LT, VT::Int, VT::Bool) { return std::get<Int>(l) < bool_to_int(std::get<Bool>(r)); };
    BINARY(OP_LT, VT::Bool, VT::Real) { return bool_to_real(std::get<Bool>(l)) < std::get<Real>(r); };
    BINARY(OP_LT, VT::Real, VT::Bool) { return std::get<Real>(l) < bool_to_real(std::get<Bool>(r)); };

    // OP_GT
    BINARY(OP_GT, VT::Int, VT::Int) { return std::get<Int>(l) > std::get<Int>(r); };
    BINARY(OP_GT, VT::Real, VT::Real) { return std::get<Real>(l) > std::get<Real>(r); };
    BINARY(OP_GT, VT::Int, VT::Real) { return static_cast<Real>(std::get<Int>(l)) > std::get<Real>(r); };
    BINARY(OP_GT, VT::Real, VT::Int) { return std::get<Real>(l) > static_cast<Real>(std::get<Int>(r)); };
    BINARY(OP_GT, VT::String, VT::String) { return std::get<String>(l)->str > std::get<String>(r)->str; };
    BINARY(OP_GT, VT::Bool, VT::Int) { return bool_to_int(std::get<Bool>(l)) > std::get<Int>(r); };
    BINARY(OP_GT, VT::Int, VT::Bool) { return std::get<Int>(l) > bool_to_int(std::get<Bool>(r)); };
    BINARY(OP_GT, VT::Bool, VT::Real) { return bool_to_real(std::get<Bool>(l)) > std::get<Real>(r); };
    BINARY(OP_GT, VT::Real, VT::Bool) { return std::get<Real>(l) > bool_to_real(std::get<Bool>(r)); };

    // OP_LE
    BINARY(OP_LE, VT::Int, VT::Int) { return std::get<Int>(l) <= std::get<Int>(r); };
    BINARY(OP_LE, VT::Real, VT::Real) { return std::get<Real>(l) <= std::get<Real>(r); };
    BINARY(OP_LE, VT::Int, VT::Real) { return static_cast<Real>(std::get<Int>(l)) <= std::get<Real>(r); };
    BINARY(OP_LE, VT::Real, VT::Int) { return std::get<Real>(l) <= static_cast<Real>(std::get<Int>(r)); };
    BINARY(OP_LE, VT::String, VT::String) { return std::get<String>(l)->str <= std::get<String>(r)->str; };
    BINARY(OP_LE, VT::Bool, VT::Int) { return bool_to_int(std::get<Bool>(l)) <= std::get<Int>(r); };
    BINARY(OP_LE, VT::Int, VT::Bool) { return std::get<Int>(l) <= bool_to_int(std::get<Bool>(r)); };
    BINARY(OP_LE, VT::Bool, VT::Real) { return bool_to_real(std::get<Bool>(l)) <= std::get<Real>(r); };
    BINARY(OP_LE, VT::Real, VT::Bool) { return std::get<Real>(l) <= bool_to_real(std::get<Bool>(r)); };

    // OP_GE
    BINARY(OP_GE, VT::Int, VT::Int) { return std::get<Int>(l) >= std::get<Int>(r); };
    BINARY(OP_GE, VT::Real, VT::Real) { return std::get<Real>(l) >= std::get<Real>(r); };
    BINARY(OP_GE, VT::Int, VT::Real) { return static_cast<Real>(std::get<Int>(l)) >= std::get<Real>(r); };
    BINARY(OP_GE, VT::Real, VT::Int) { return std::get<Real>(l) >= static_cast<Real>(std::get<Int>(r)); };
    BINARY(OP_GE, VT::String, VT::String) { return std::get<String>(l)->str >= std::get<String>(r)->str; };
    BINARY(OP_GE, VT::Bool, VT::Int) { return bool_to_int(std::get<Bool>(l)) >= std::get<Int>(r); };
    BINARY(OP_GE, VT::Int, VT::Bool) { return std::get<Int>(l) >= bool_to_int(std::get<Bool>(r)); };
    BINARY(OP_GE, VT::Bool, VT::Real) { return bool_to_real(std::get<Bool>(l)) >= std::get<Real>(r); };
    BINARY(OP_GE, VT::Real, VT::Bool) { return std::get<Real>(l) >= bool_to_real(std::get<Bool>(r)); };

    // BITWISE
    BINARY(OP_BIT_AND, VT::Int, VT::Int) { return std::get<Int>(l) & std::get<Int>(r); };
    BINARY(OP_BIT_AND, VT::Int, VT::String) { return std::get<Int>(l) & int_from_char(std::get<String>(r)); };
    BINARY(OP_BIT_AND, VT::String, VT::Int) { return int_from_char(std::get<String>(l)) & std::get<Int>(r); };
    BINARY(OP_BIT_OR, VT::Int, VT::Int) { return std::get<Int>(l) | std::get<Int>(r); };
    BINARY(OP_BIT_OR, VT::Int, VT::String) { return std::get<Int>(l) | int_from_char(std::get<String>(r)); };
    BINARY(OP_BIT_OR, VT::String, VT::Int) { return int_from_char(std::get<String>(l)) | std::get<Int>(r); };
    BINARY(OP_BIT_XOR, VT::Int, VT::Int) { return std::get<Int>(l) ^ std::get<Int>(r); };
    BINARY(OP_BIT_XOR, VT::Int, VT::String) { return std::get<Int>(l) ^ int_from_char(std::get<String>(r)); };
    BINARY(OP_BIT_XOR, VT::String, VT::Int) { return int_from_char(std::get<String>(l)) ^ std::get<Int>(r); };
    BINARY(OP_LSHIFT, VT::Int, VT::Int) { return std::get<Int>(l) << std::get<Int>(r); };
    BINARY(OP_LSHIFT, VT::Int, VT::String) { return std::get<Int>(l) << int_from_char(std::get<String>(r)); };
    BINARY(OP_LSHIFT, VT::String, VT::Int) { return int_from_char(std::get<String>(l)) << std::get<Int>(r); };
    BINARY(OP_RSHIFT, VT::Int, VT::Int) { return std::get<Int>(l) >> std::get<Int>(r); };
    BINARY(OP_RSHIFT, VT::Int, VT::String) { return std::get<Int>(l) >> int_from_char(std::get<String>(r)); };
    BINARY(OP_RSHIFT, VT::String, VT::Int) { return int_from_char(std::get<String>(l)) >> std::get<Int>(r); };
    BINARY(OP_BIT_AND, VT::Bool, VT::Bool) { return static_cast<Int>(std::get<Bool>(l)) & static_cast<Int>(std::get<Bool>(r)); };
    BINARY(OP_BIT_OR, VT::Bool, VT::Bool) { return static_cast<Int>(std::get<Bool>(l)) | static_cast<Int>(std::get<Bool>(r)); };

    // UNARY OPS
    UNARY(OP_MINUS, VT::Int) { return -std::get<Int>(r); };
    UNARY(OP_MINUS, VT::Real) { return -std::get<Real>(r); };
    UNARY(OP_MINUS, VT::String) { return -int_from_char(std::get<String>(r)); };
    UNARY(OP_BIT_NOT, VT::Int) { return ~std::get<Int>(r); };
    UNARY(OP_BIT_NOT, VT::String) { return ~int_from_char(std::get<String>(r)); };

    UNARY(OP_LOGICAL_NOT, VT::Null) { return true; };
    UNARY(OP_LOGICAL_NOT, VT::Bool) { return !std::get<Bool>(r); };
    UNARY(OP_LOGICAL_NOT, VT::Int) { return std::get<Int>(r) == 0; };
    UNARY(OP_LOGICAL_NOT, VT::Real) { return std::get<Real>(r) == 0.0; };
    UNARY(OP_LOGICAL_NOT, VT::String) { return std::get<String>(r)->str.empty(); };
    UNARY(OP_LOGICAL_NOT, VT::Array) { return std::get<Array>(r)->elements.empty(); };
    UNARY(OP_LOGICAL_NOT, VT::Object) { return std::get<Object>(r)->pairs.empty(); };
    UNARY(OP_LOGICAL_NOT, VT::Function) { return false; };
    UNARY(OP_LOGICAL_NOT, VT::Instance) { return false; };
    UNARY(OP_LOGICAL_NOT, VT::Class) { return false; };
    UNARY(OP_LOGICAL_NOT, VT::BoundMethod) { return false; };
}