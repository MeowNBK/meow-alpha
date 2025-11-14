#include "runtime/operator_dispatcher.hpp"
#include "runtime/overload.hpp"
#include <stdexcept>
#include <cmath>

ValueType getValueType(const Value& value) {
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

std::string getBinaryOperatorMethodName(TokenType opType) {
    using enum TokenType;
    switch (opType) {
        case OP_PLUS: return "__add__";
        case OP_MINUS: return "__sub__";
        case OP_MULTIPLY: return "__mul__";
        case OP_DIVIDE: return "__div__";
        case OP_MODULO: return "__mod__";
        case OP_EXPONENT: return "__power__";
        case OP_EQ: return "__eq__";
        case OP_NEQ: return "__neq__";
        case OP_LT: return "__lt__";
        case OP_GT: return "__gt__";
        case OP_LE: return "__le__";
        case OP_GE: return "__ge__";
        case OP_BIT_AND: return "__band__";
        case OP_BIT_OR: return "__bor__";
        case OP_BIT_XOR: return "__bxor__";
        case OP_LSHIFT: return "__lshift__";
        case OP_RSHIFT: return "__rshift__";
        default: return "";
    }
}

std::string getUnaryOperatorMethodName(TokenType opType) {
    using enum TokenType;
    switch (opType) {
        case OP_MINUS: return "__neg__";
        case OP_LOGICAL_NOT: return "__not__";
        case OP_BIT_NOT: return "__bnot__";
        default: return "";
    }
}

OperatorDispatcher::OperatorDispatcher() {
    using enum TokenType;
    using VT = ValueType;

    auto intFromChar = [](const String& s) {
        if (s->str.length() != 1) {
            throw std::runtime_error("Cannot convert multi-character string to Int.");
        }
        return static_cast<Int>(s->str[0]);
    };

    auto boolToInt = [](const Bool& b) {
        return static_cast<Int>(b);
    };

    auto boolToReal = [](const Bool& b) {
        return static_cast<Real>(b);
    };

    binaryOps[{OP_PLUS, VT::Int, VT::Int}] = [](const Value& l, const Value& r) { return std::get<Int>(l) + std::get<Int>(r); };
    binaryOps[{OP_PLUS, VT::Real, VT::Real}] = [](const Value& l, const Value& r) { return std::get<Real>(l) + std::get<Real>(r); };
    binaryOps[{OP_PLUS, VT::Int, VT::Real}] = [](const Value& l, const Value& r) { return static_cast<Real>(std::get<Int>(l)) + std::get<Real>(r); };
    binaryOps[{OP_PLUS, VT::Real, VT::Int}] = [](const Value& l, const Value& r) { return std::get<Real>(l) + static_cast<Real>(std::get<Int>(r)); };
    binaryOps[{OP_PLUS, VT::String, VT::String}] = [](const Value& l, const Value& r) { return Value(std::get<String>(l)->str + std::get<String>(r)->str); };
    binaryOps[{OP_PLUS, VT::String, VT::Int}] = [](const Value& l, const Value& r) { return Value(std::get<String>(l)->str + std::to_string(std::get<Int>(r))); };
    binaryOps[{OP_PLUS, VT::Int, VT::String}] = [](const Value& l, const Value& r) { return Value(std::to_string(std::get<Int>(l)) + std::get<String>(r)->str); };
    binaryOps[{OP_PLUS, VT::String, VT::Real}] = [](const Value& l, const Value& r) { return Value(std::get<String>(l)->str + std::to_string(std::get<Real>(r))); };
    binaryOps[{OP_PLUS, VT::Real, VT::String}] = [](const Value& l, const Value& r) { return Value(std::to_string(std::get<Real>(l)) + std::get<String>(r)->str); };
    binaryOps[{OP_PLUS, VT::Int, VT::Bool}] = [boolToInt](const Value& l, const Value& r) { return std::get<Int>(l) + boolToInt(std::get<Bool>(r)); };
    binaryOps[{OP_PLUS, VT::Bool, VT::Int}] = [boolToInt](const Value& l, const Value& r) { return boolToInt(std::get<Bool>(l)) + std::get<Int>(r); };
    binaryOps[{OP_PLUS, VT::Real, VT::Bool}] = [boolToReal](const Value& l, const Value& r) { return std::get<Real>(l) + boolToReal(std::get<Bool>(r)); };
    binaryOps[{OP_PLUS, VT::Bool, VT::Real}] = [boolToReal](const Value& l, const Value& r) { return boolToReal(std::get<Bool>(l)) + std::get<Real>(r); };
    binaryOps[{OP_PLUS, VT::String, VT::Null}] = [](const Value& l, const Value& r) {
        return Value(std::get<String>(l)->str + toString(r));
    };
    binaryOps[{OP_PLUS, VT::Null, VT::String}] = [](const Value& l, const Value& r) {
        return Value(toString(l) + std::get<String>(r)->str);
    };
    binaryOps[{OP_PLUS, VT::String, VT::Array}] = [](const Value& l, const Value& r) {
        return Value(std::get<String>(l)->str + toString(r));
    };
    binaryOps[{OP_PLUS, VT::Array, VT::String}] = [](const Value& l, const Value& r) {
        return Value(toString(l) + std::get<String>(r)->str);
    };
    binaryOps[{OP_PLUS, VT::String, VT::Object}] = [](const Value& l, const Value& r) {
        return Value(std::get<String>(l)->str + toString(r));
    };
    binaryOps[{OP_PLUS, VT::Object, VT::String}] = [](const Value& l, const Value& r) {
        return Value(toString(l) + std::get<String>(r)->str);
    };
    binaryOps[{OP_PLUS, VT::Array, VT::Array}] = [](const Value& l, const Value& r) {
        Array newArr = std::make_shared<ArrayData>();
        newArr->elements = std::get<Array>(l)->elements;
        const auto& r_elements = std::get<Array>(r)->elements;
        newArr->elements.insert(newArr->elements.end(), r_elements.begin(), r_elements.end());
        return Value(newArr);
    };

    binaryOps[{OP_MINUS, VT::Int, VT::Int}] = [](const Value& l, const Value& r) { return std::get<Int>(l) - std::get<Int>(r); };
    binaryOps[{OP_MINUS, VT::Real, VT::Real}] = [](const Value& l, const Value& r) { return std::get<Real>(l) - std::get<Real>(r); };
    binaryOps[{OP_MINUS, VT::Int, VT::Real}] = [](const Value& l, const Value& r) { return static_cast<Real>(std::get<Int>(l)) - std::get<Real>(r); };
    binaryOps[{OP_MINUS, VT::Real, VT::Int}] = [](const Value& l, const Value& r) { return std::get<Real>(l) - static_cast<Real>(std::get<Int>(r)); };
    binaryOps[{OP_MINUS, VT::Int, VT::Bool}] = [boolToInt](const Value& l, const Value& r) { return std::get<Int>(l) - boolToInt(std::get<Bool>(r)); };
    binaryOps[{OP_MINUS, VT::Bool, VT::Int}] = [boolToInt](const Value& l, const Value& r) { return boolToInt(std::get<Bool>(l)) - std::get<Int>(r); };
    binaryOps[{OP_MINUS, VT::Real, VT::Bool}] = [boolToReal](const Value& l, const Value& r) { return std::get<Real>(l) - boolToReal(std::get<Bool>(r)); };
    binaryOps[{OP_MINUS, VT::Bool, VT::Real}] = [boolToReal](const Value& l, const Value& r) { return boolToReal(std::get<Bool>(l)) - std::get<Real>(r); };

    binaryOps[{OP_MULTIPLY, VT::Int, VT::Int}] = [](const Value& l, const Value& r) { return std::get<Int>(l) * std::get<Int>(r); };
    binaryOps[{OP_MULTIPLY, VT::Real, VT::Real}] = [](const Value& l, const Value& r) { return std::get<Real>(l) * std::get<Real>(r); };
    binaryOps[{OP_MULTIPLY, VT::Int, VT::Real}] = [](const Value& l, const Value& r) { return static_cast<Real>(std::get<Int>(l)) * std::get<Real>(r); };
    binaryOps[{OP_MULTIPLY, VT::Real, VT::Int}] = [](const Value& l, const Value& r) { return std::get<Real>(l) * static_cast<Real>(std::get<Int>(r)); };
    binaryOps[{OP_MULTIPLY, VT::String, VT::Int}] = [](const Value& l, const Value& r) {
        std::string res;
        for (Int i = 0; i < std::get<Int>(r); ++i) res += std::get<String>(l)->str;
        return Value(res);
    };
    binaryOps[{OP_MULTIPLY, VT::Int, VT::String}] = [](const Value& l, const Value& r) {
        std::string res;
        for (Int i = 0; i < std::get<Int>(l); ++i) res += std::get<String>(r)->str;
        return Value(res);
    };
    binaryOps[{OP_MULTIPLY, VT::Int, VT::Bool}] = [boolToInt](const Value& l, const Value& r) { return std::get<Int>(l) * boolToInt(std::get<Bool>(r)); };
    binaryOps[{OP_MULTIPLY, VT::Bool, VT::Int}] = [boolToInt](const Value& l, const Value& r) { return boolToInt(std::get<Bool>(l)) * std::get<Int>(r); };
    binaryOps[{OP_MULTIPLY, VT::Real, VT::Bool}] = [boolToReal](const Value& l, const Value& r) { return std::get<Real>(l) * boolToReal(std::get<Bool>(r)); };
    binaryOps[{OP_MULTIPLY, VT::Bool, VT::Real}] = [boolToReal](const Value& l, const Value& r) { return boolToReal(std::get<Bool>(l)) * std::get<Real>(r); };
    binaryOps[{OP_MULTIPLY, VT::Array, VT::Int}] = [](const Value& l, const Value& r) {
        const auto& originalElements = std::get<Array>(l)->elements;
        Int multiplier = std::get<Int>(r);
        Array newArray = std::make_shared<ArrayData>();
        if (multiplier <= 0) return Value(newArray);
        newArray->elements.reserve(originalElements.size() * multiplier);
        for (Int i = 0; i < multiplier; ++i) {
            newArray->elements.insert(newArray->elements.end(), originalElements.begin(), originalElements.end());
        }
        return Value(newArray);
    };
    binaryOps[{OP_MULTIPLY, VT::Int, VT::Array}] = [this](const Value& l, const Value& r) {
        return binaryOps.at(BinaryOperationKey{OP_MULTIPLY, VT::Array, VT::Int})(r, l);
    };

    binaryOps[{OP_DIVIDE, VT::Int, VT::Int}] = [](const Value& l, const Value& r) {
        if (std::get<Int>(r) == 0) {
            if (std::get<Int>(l) == 0) return std::numeric_limits<Real>::quiet_NaN();
            return (std::get<Int>(l) > 0) ? std::numeric_limits<Real>::infinity() : -std::numeric_limits<Real>::infinity();
        }
        return static_cast<Real>(std::get<Int>(l)) / static_cast<Real>(std::get<Int>(r));
    };
    binaryOps[{OP_DIVIDE, VT::Real, VT::Real}] = [](const Value& l, const Value& r) {
        return std::get<Real>(l) / std::get<Real>(r);
    };
    binaryOps[{OP_DIVIDE, VT::Int, VT::Real}] = [](const Value& l, const Value& r) {
        if (std::get<Real>(r) == 0.0) {
            if (std::get<Int>(l) == 0) return std::numeric_limits<Real>::quiet_NaN();
            return (std::get<Int>(l) > 0) ? std::numeric_limits<Real>::infinity() : -std::numeric_limits<Real>::infinity();
        }
        return static_cast<Real>(std::get<Int>(l)) / std::get<Real>(r);
    };
    binaryOps[{OP_DIVIDE, VT::Real, VT::Int}] = [](const Value& l, const Value& r) {
        if (std::get<Int>(r) == 0) {
            if (std::get<Real>(l) == 0.0) return std::numeric_limits<Real>::quiet_NaN();
            return (std::get<Real>(l) > 0.0) ? std::numeric_limits<Real>::infinity() : -std::numeric_limits<Real>::infinity();
        }
        return std::get<Real>(l) / static_cast<Real>(std::get<Int>(r));
    };
    binaryOps[{OP_DIVIDE, VT::Int, VT::Bool}] = [boolToInt](const Value& l, const Value& r) {
        Int divisor = boolToInt(std::get<Bool>(r));
        if (divisor == 0) {
            if (std::get<Int>(l) == 0) return std::numeric_limits<Real>::quiet_NaN();
            return (std::get<Int>(l) > 0) ? std::numeric_limits<Real>::infinity() : -std::numeric_limits<Real>::infinity();
        }
        return static_cast<Real>(std::get<Int>(l)) / static_cast<Real>(divisor);
    };
    binaryOps[{OP_DIVIDE, VT::Real, VT::Bool}] = [boolToReal](const Value& l, const Value& r) {
        Real divisor = boolToReal(std::get<Bool>(r));
        if (divisor == 0.0) {
            if (std::get<Real>(l) == 0.0) return std::numeric_limits<Real>::quiet_NaN();
            return (std::get<Real>(l) > 0.0) ? std::numeric_limits<Real>::infinity() : -std::numeric_limits<Real>::infinity();
        }
        return std::get<Real>(l) / divisor;
    };

    binaryOps[{OP_MODULO, VT::Int, VT::Int}] = [](const Value& l, const Value& r) {
        if (std::get<Int>(r) == 0) throw std::runtime_error("Modulo cho 0.");
        return std::get<Int>(l) % std::get<Int>(r);
    };
    binaryOps[{OP_MODULO, VT::Int, VT::Bool}] = [boolToInt](const Value& l, const Value& r) {
        Int divisor = boolToInt(std::get<Bool>(r));
        if (divisor == 0) throw std::runtime_error("Modulo cho 0.");
        return std::get<Int>(l) % divisor;
    };

    binaryOps[{OP_EXPONENT, VT::Int, VT::Int}] = [](const Value& l, const Value& r) { 
        return std::pow(static_cast<Real>(std::get<Int>(l)), static_cast<Real>(std::get<Int>(r))); 
    };
    binaryOps[{OP_EXPONENT, VT::Real, VT::Real}] = [](const Value& l, const Value& r) { 
        return std::pow(std::get<Real>(l), std::get<Real>(r)); 
    };
    binaryOps[{OP_EXPONENT, VT::Int, VT::Real}] = [](const Value& l, const Value& r) { 
        return std::pow(static_cast<Real>(std::get<Int>(l)), std::get<Real>(r)); 
    };
    binaryOps[{OP_EXPONENT, VT::Real, VT::Int}] = [](const Value& l, const Value& r) { 
        return std::pow(std::get<Real>(l), static_cast<Real>(std::get<Int>(r))); 
    };

    binaryOps[{OP_EQ, VT::Int, VT::Int}] = [](const Value& l, const Value& r) { return std::get<Int>(l) == std::get<Int>(r); };
    binaryOps[{OP_EQ, VT::Real, VT::Real}] = [](const Value& l, const Value& r) { return std::get<Real>(l) == std::get<Real>(r); };
    binaryOps[{OP_EQ, VT::Int, VT::Real}] = [](const Value& l, const Value& r) { return static_cast<Real>(std::get<Int>(l)) == std::get<Real>(r); };
    binaryOps[{OP_EQ, VT::Real, VT::Int}] = [](const Value& l, const Value& r) { return std::get<Real>(l) == static_cast<Real>(std::get<Int>(r)); };
    binaryOps[{OP_EQ, VT::Bool, VT::Bool}] = [](const Value& l, const Value& r) { return std::get<Bool>(l) == std::get<Bool>(r); };
    binaryOps[{OP_EQ, VT::String, VT::String}] = [](const Value& l, const Value& r) { return std::get<String>(l)->str == std::get<String>(r)->str; };
    binaryOps[{OP_EQ, VT::Null, VT::Null}] = [](const Value&, const Value&) { return true; };
    binaryOps[{OP_EQ, VT::Bool, VT::Int}] = [boolToInt](const Value& l, const Value& r) { return boolToInt(std::get<Bool>(l)) == std::get<Int>(r); };
    binaryOps[{OP_EQ, VT::Int, VT::Bool}] = [boolToInt](const Value& l, const Value& r) { return std::get<Int>(l) == boolToInt(std::get<Bool>(r)); };
    binaryOps[{OP_EQ, VT::Null, VT::String}] = [](const Value&, const Value& r) { return std::get<String>(r)->str.empty(); };
    binaryOps[{OP_EQ, VT::String, VT::Null}] = [](const Value& l, const Value&) { return std::get<String>(l)->str.empty(); };
    binaryOps[{OP_EQ, VT::Null, VT::Bool}] = [](const Value&, const Value& r) { return std::get<Bool>(r) == false; };
    binaryOps[{OP_EQ, VT::Bool, VT::Null}] = [](const Value& l, const Value&) { return std::get<Bool>(l) == false; };
    binaryOps[{OP_EQ, VT::Null, VT::Int}] = [](const Value&, const Value& r) { 

        return false;
    };
    binaryOps[{OP_EQ, VT::Int, VT::Null}] = [](const Value& l, const Value&) {

        return false;
    };
    binaryOps[{OP_EQ, VT::Null, VT::Real}] = [](const Value&, const Value& r) { 

        return false;
    };
    binaryOps[{OP_EQ, VT::Real, VT::Null}] = [](const Value& l, const Value&) { 

        return false;
    };
    binaryOps[{OP_EQ, VT::Null, VT::Array}] = [](const Value&, const Value& r) { return std::get<Array>(r)->elements.empty(); };
    binaryOps[{OP_EQ, VT::Array, VT::Null}] = [](const Value& l, const Value&) { return std::get<Array>(l)->elements.empty(); };
    binaryOps[{OP_EQ, VT::Null, VT::Object}] = [](const Value&, const Value& r) { return std::get<Object>(r)->pairs.empty(); };
    binaryOps[{OP_EQ, VT::Object, VT::Null}] = [](const Value& l, const Value&) { return std::get<Object>(l)->pairs.empty(); };
    binaryOps[{OP_EQ, VT::Null, VT::Function}] = [](const Value&, const Value&) { return false; };
    binaryOps[{OP_EQ, VT::Function, VT::Null}] = [](const Value&, const Value&) { return false; };

    binaryOps[{OP_EQ, VT::Null, VT::Instance}] = [](const Value&, const Value&) { return false; };
    binaryOps[{OP_EQ, VT::Instance, VT::Null}] = [](const Value&, const Value&) { return false; };

    binaryOps[{OP_EQ, VT::Null, VT::Class}] = [](const Value&, const Value&) { return false; };
    binaryOps[{OP_EQ, VT::Class, VT::Null}] = [](const Value&, const Value&) { return false; };

    binaryOps[{OP_EQ, VT::Null, VT::BoundMethod}] = [](const Value&, const Value&) { return false; };
    binaryOps[{OP_EQ, VT::BoundMethod, VT::Null}] = [](const Value&, const Value&) { return false; };

    binaryOps[{OP_NEQ, VT::Int, VT::Int}] = [](const Value& l, const Value& r) { return std::get<Int>(l) != std::get<Int>(r); };
    binaryOps[{OP_NEQ, VT::Real, VT::Real}] = [](const Value& l, const Value& r) { return std::get<Real>(l) != std::get<Real>(r); };
    binaryOps[{OP_NEQ, VT::Int, VT::Real}] = [](const Value& l, const Value& r) { return static_cast<Real>(std::get<Int>(l)) != std::get<Real>(r); };
    binaryOps[{OP_NEQ, VT::Real, VT::Int}] = [](const Value& l, const Value& r) { return std::get<Real>(l) != static_cast<Real>(std::get<Int>(r)); };
    binaryOps[{OP_NEQ, VT::Bool, VT::Bool}] = [](const Value& l, const Value& r) { return std::get<Bool>(l) != std::get<Bool>(r); };
    binaryOps[{OP_NEQ, VT::String, VT::String}] = [](const Value& l, const Value& r) { return std::get<String>(l)->str != std::get<String>(r)->str; };
    binaryOps[{OP_NEQ, VT::Null, VT::Null}] = [](const Value&, const Value&) { return false; };
    binaryOps[{OP_NEQ, VT::Bool, VT::Int}] = [boolToInt](const Value& l, const Value& r) { return boolToInt(std::get<Bool>(l)) != std::get<Int>(r); };
    binaryOps[{OP_NEQ, VT::Int, VT::Bool}] = [boolToInt](const Value& l, const Value& r) { return std::get<Int>(l) != boolToInt(std::get<Bool>(r)); };
    binaryOps[{OP_NEQ, VT::Null, VT::String}] = [](const Value&, const Value& r) { return !std::get<String>(r)->str.empty(); };
    binaryOps[{OP_NEQ, VT::String, VT::Null}] = [](const Value& l, const Value&) { return !std::get<String>(l)->str.empty(); };
    binaryOps[{OP_NEQ, VT::Null, VT::Bool}] = [](const Value&, const Value& r) { return std::get<Bool>(r) == true; };
    binaryOps[{OP_NEQ, VT::Bool, VT::Null}] = [](const Value& l, const Value&) { return std::get<Bool>(l) == true; };
    binaryOps[{OP_NEQ, VT::Null, VT::Int}] = [](const Value&, const Value& r) { 

        return true;
    };
    binaryOps[{OP_NEQ, VT::Int, VT::Null}] = [](const Value& l, const Value&) { 

        return true;
    };
    binaryOps[{OP_NEQ, VT::Null, VT::Real}] = [](const Value&, const Value& r) { 

        return true;
    };
    binaryOps[{OP_NEQ, VT::Real, VT::Null}] = [](const Value& l, const Value&) { 

        return true;
    };
    binaryOps[{OP_NEQ, VT::Null, VT::Array}] = [](const Value&, const Value& r) { return !std::get<Array>(r)->elements.empty(); };
    binaryOps[{OP_NEQ, VT::Array, VT::Null}] = [](const Value& l, const Value&) { return !std::get<Array>(l)->elements.empty(); };
    binaryOps[{OP_NEQ, VT::Null, VT::Object}] = [](const Value&, const Value& r) { return !std::get<Object>(r)->pairs.empty(); };
    binaryOps[{OP_NEQ, VT::Object, VT::Null}] = [](const Value& l, const Value&) { return !std::get<Object>(l)->pairs.empty(); };
    binaryOps[{OP_NEQ, VT::Null, VT::Function}] = [](const Value&, const Value&) { return true; };
    binaryOps[{OP_NEQ, VT::Function, VT::Null}] = [](const Value&, const Value&) { return true; };

    binaryOps[{OP_NEQ, VT::Null, VT::Instance}] = [](const Value&, const Value&) { return true; };
    binaryOps[{OP_NEQ, VT::Instance, VT::Null}] = [](const Value&, const Value&) { return true; };

    binaryOps[{OP_NEQ, VT::Null, VT::Class}] = [](const Value&, const Value&) { return true; };
    binaryOps[{OP_NEQ, VT::Class, VT::Null}] = [](const Value&, const Value&) { return true; };

    binaryOps[{OP_NEQ, VT::Null, VT::BoundMethod}] = [](const Value&, const Value&) { return true; };
    binaryOps[{OP_NEQ, VT::BoundMethod, VT::Null}] = [](const Value&, const Value&) { return true; };

    binaryOps[{OP_LT, VT::Int, VT::Int}] = [](const Value& l, const Value& r) { return std::get<Int>(l) < std::get<Int>(r); };
    binaryOps[{OP_LT, VT::Real, VT::Real}] = [](const Value& l, const Value& r) { return std::get<Real>(l) < std::get<Real>(r); };
    binaryOps[{OP_LT, VT::Int, VT::Real}] = [](const Value& l, const Value& r) { return static_cast<Real>(std::get<Int>(l)) < std::get<Real>(r); };
    binaryOps[{OP_LT, VT::Real, VT::Int}] = [](const Value& l, const Value& r) { return std::get<Real>(l) < static_cast<Real>(std::get<Int>(r)); };
    binaryOps[{OP_LT, VT::String, VT::String}] = [](const Value& l, const Value& r) { return std::get<String>(l)->str < std::get<String>(r)->str; };
    binaryOps[{OP_LT, VT::Bool, VT::Int}] = [boolToInt](const Value& l, const Value& r) { return boolToInt(std::get<Bool>(l)) < std::get<Int>(r); };
    binaryOps[{OP_LT, VT::Int, VT::Bool}] = [boolToInt](const Value& l, const Value& r) { return std::get<Int>(l) < boolToInt(std::get<Bool>(r)); };
    binaryOps[{OP_LT, VT::Bool, VT::Real}] = [boolToReal](const Value& l, const Value& r) { return boolToReal(std::get<Bool>(l)) < std::get<Real>(r); };
    binaryOps[{OP_LT, VT::Real, VT::Bool}] = [boolToReal](const Value& l, const Value& r) { return std::get<Real>(l) < boolToReal(std::get<Bool>(r)); };

    binaryOps[{OP_GT, VT::Int, VT::Int}] = [](const Value& l, const Value& r) { return std::get<Int>(l) > std::get<Int>(r); };
    binaryOps[{OP_GT, VT::Real, VT::Real}] = [](const Value& l, const Value& r) { return std::get<Real>(l) > std::get<Real>(r); };
    binaryOps[{OP_GT, VT::Int, VT::Real}] = [](const Value& l, const Value& r) { return static_cast<Real>(std::get<Int>(l)) > std::get<Real>(r); };
    binaryOps[{OP_GT, VT::Real, VT::Int}] = [](const Value& l, const Value& r) { return std::get<Real>(l) > static_cast<Real>(std::get<Int>(r)); };
    binaryOps[{OP_GT, VT::String, VT::String}] = [](const Value& l, const Value& r) { return std::get<String>(l)->str > std::get<String>(r)->str; };
    binaryOps[{OP_GT, VT::Bool, VT::Int}] = [boolToInt](const Value& l, const Value& r) { return boolToInt(std::get<Bool>(l)) > std::get<Int>(r); };
    binaryOps[{OP_GT, VT::Int, VT::Bool}] = [boolToInt](const Value& l, const Value& r) { return std::get<Int>(l) > boolToInt(std::get<Bool>(r)); };
    binaryOps[{OP_GT, VT::Bool, VT::Real}] = [boolToReal](const Value& l, const Value& r) { return boolToReal(std::get<Bool>(l)) > std::get<Real>(r); };
    binaryOps[{OP_GT, VT::Real, VT::Bool}] = [boolToReal](const Value& l, const Value& r) { return std::get<Real>(l) > boolToReal(std::get<Bool>(r)); };

    binaryOps[{OP_LE, VT::Int, VT::Int}] = [](const Value& l, const Value& r) { return std::get<Int>(l) <= std::get<Int>(r); };
    binaryOps[{OP_LE, VT::Real, VT::Real}] = [](const Value& l, const Value& r) { return std::get<Real>(l) <= std::get<Real>(r); };
    binaryOps[{OP_LE, VT::Int, VT::Real}] = [](const Value& l, const Value& r) { return static_cast<Real>(std::get<Int>(l)) <= std::get<Real>(r); };
    binaryOps[{OP_LE, VT::Real, VT::Int}] = [](const Value& l, const Value& r) { return std::get<Real>(l) <= static_cast<Real>(std::get<Int>(r)); };
    binaryOps[{OP_LE, VT::String, VT::String}] = [](const Value& l, const Value& r) { return std::get<String>(l)->str <= std::get<String>(r)->str; };
    binaryOps[{OP_LE, VT::Bool, VT::Int}] = [boolToInt](const Value& l, const Value& r) { return boolToInt(std::get<Bool>(l)) <= std::get<Int>(r); };
    binaryOps[{OP_LE, VT::Int, VT::Bool}] = [boolToInt](const Value& l, const Value& r) { return std::get<Int>(l) <= boolToInt(std::get<Bool>(r)); };
    binaryOps[{OP_LE, VT::Bool, VT::Real}] = [boolToReal](const Value& l, const Value& r) { return boolToReal(std::get<Bool>(l)) <= std::get<Real>(r); };
    binaryOps[{OP_LE, VT::Real, VT::Bool}] = [boolToReal](const Value& l, const Value& r) { return std::get<Real>(l) <= boolToReal(std::get<Bool>(r)); };

    binaryOps[{OP_GE, VT::Int, VT::Int}] = [](const Value& l, const Value& r) { return std::get<Int>(l) >= std::get<Int>(r); };
    binaryOps[{OP_GE, VT::Real, VT::Real}] = [](const Value& l, const Value& r) { return std::get<Real>(l) >= std::get<Real>(r); };
    binaryOps[{OP_GE, VT::Int, VT::Real}] = [](const Value& l, const Value& r) { return static_cast<Real>(std::get<Int>(l)) >= std::get<Real>(r); };
    binaryOps[{OP_GE, VT::Real, VT::Int}] = [](const Value& l, const Value& r) { return std::get<Real>(l) >= static_cast<Real>(std::get<Int>(r)); };
    binaryOps[{OP_GE, VT::String, VT::String}] = [](const Value& l, const Value& r) { return std::get<String>(l)->str >= std::get<String>(r)->str; };
    binaryOps[{OP_GE, VT::Bool, VT::Int}] = [boolToInt](const Value& l, const Value& r) { return boolToInt(std::get<Bool>(l)) >= std::get<Int>(r); };
    binaryOps[{OP_GE, VT::Int, VT::Bool}] = [boolToInt](const Value& l, const Value& r) { return std::get<Int>(l) >= boolToInt(std::get<Bool>(r)); };
    binaryOps[{OP_GE, VT::Bool, VT::Real}] = [boolToReal](const Value& l, const Value& r) { return boolToReal(std::get<Bool>(l)) >= std::get<Real>(r); };
    binaryOps[{OP_GE, VT::Real, VT::Bool}] = [boolToReal](const Value& l, const Value& r) { return std::get<Real>(l) >= boolToReal(std::get<Bool>(r)); };

    binaryOps[{OP_BIT_AND, VT::Int, VT::Int}] = [](const Value& l, const Value& r) { return std::get<Int>(l) & std::get<Int>(r); };
    binaryOps[{OP_BIT_AND, VT::Int, VT::String}] = [intFromChar](const Value& l, const Value& r) { return std::get<Int>(l) & intFromChar(std::get<String>(r)); };
    binaryOps[{OP_BIT_AND, VT::String, VT::Int}] = [intFromChar](const Value& l, const Value& r) { return intFromChar(std::get<String>(l)) & std::get<Int>(r); };
    binaryOps[{OP_BIT_OR, VT::Int, VT::Int}] = [](const Value& l, const Value& r) { return std::get<Int>(l) | std::get<Int>(r); };
    binaryOps[{OP_BIT_OR, VT::Int, VT::String}] = [intFromChar](const Value& l, const Value& r) { return std::get<Int>(l) | intFromChar(std::get<String>(r)); };
    binaryOps[{OP_BIT_OR, VT::String, VT::Int}] = [intFromChar](const Value& l, const Value& r) { return intFromChar(std::get<String>(l)) | std::get<Int>(r); };
    binaryOps[{OP_BIT_XOR, VT::Int, VT::Int}] = [](const Value& l, const Value& r) { return std::get<Int>(l) ^ std::get<Int>(r); };
    binaryOps[{OP_BIT_XOR, VT::Int, VT::String}] = [intFromChar](const Value& l, const Value& r) { return std::get<Int>(l) ^ intFromChar(std::get<String>(r)); };
    binaryOps[{OP_BIT_XOR, VT::String, VT::Int}] = [intFromChar](const Value& l, const Value& r) { return intFromChar(std::get<String>(l)) ^ std::get<Int>(r); };
    binaryOps[{OP_LSHIFT, VT::Int, VT::Int}] = [](const Value& l, const Value& r) { return std::get<Int>(l) << std::get<Int>(r); };
    binaryOps[{OP_LSHIFT, VT::Int, VT::String}] = [intFromChar](const Value& l, const Value& r) { return std::get<Int>(l) << intFromChar(std::get<String>(r)); };
    binaryOps[{OP_LSHIFT, VT::String, VT::Int}] = [intFromChar](const Value& l, const Value& r) { return intFromChar(std::get<String>(l)) << std::get<Int>(r); };
    binaryOps[{OP_RSHIFT, VT::Int, VT::Int}] = [](const Value& l, const Value& r) { return std::get<Int>(l) >> std::get<Int>(r); };
    binaryOps[{OP_RSHIFT, VT::Int, VT::String}] = [intFromChar](const Value& l, const Value& r) { return std::get<Int>(l) >> intFromChar(std::get<String>(r)); };
    binaryOps[{OP_RSHIFT, VT::String, VT::Int}] = [intFromChar](const Value& l, const Value& r) { return intFromChar(std::get<String>(l)) >> std::get<Int>(r); };
    binaryOps[{OP_BIT_AND, VT::Bool, VT::Bool}] = [](const Value& l, const Value& r) { return static_cast<Int>(std::get<Bool>(l)) & static_cast<Int>(std::get<Bool>(r)); };
    binaryOps[{OP_BIT_OR, VT::Bool, VT::Bool}] = [](const Value& l, const Value& r) { return static_cast<Int>(std::get<Bool>(l)) | static_cast<Int>(std::get<Bool>(r)); };

    unaryOps[{OP_MINUS, VT::Int}] = [](const Value& r) { return -std::get<Int>(r); };
    unaryOps[{OP_MINUS, VT::Real}] = [](const Value& r) { return -std::get<Real>(r); };
    unaryOps[{OP_MINUS, VT::String}] = [intFromChar](const Value& r) { return -intFromChar(std::get<String>(r)); };
    unaryOps[{OP_BIT_NOT, VT::Int}] = [](const Value& r) { return ~std::get<Int>(r); };
    unaryOps[{OP_BIT_NOT, VT::String}] = [intFromChar](const Value& r) { return ~intFromChar(std::get<String>(r)); };

    unaryOps[{OP_LOGICAL_NOT, VT::Null}] = [](const Value&) { return true; };
    unaryOps[{OP_LOGICAL_NOT, VT::Bool}] = [](const Value& r) { return !std::get<Bool>(r); };
    unaryOps[{OP_LOGICAL_NOT, VT::Int}] = [](const Value& r) { return std::get<Int>(r) == 0; };
    unaryOps[{OP_LOGICAL_NOT, VT::Real}] = [](const Value& r) { return std::get<Real>(r) == 0.0; };
    unaryOps[{OP_LOGICAL_NOT, VT::String}] = [](const Value& r) { return std::get<String>(r)->str.empty(); };
    unaryOps[{OP_LOGICAL_NOT, VT::Array}] = [](const Value& r) { return std::get<Array>(r)->elements.empty(); };
    unaryOps[{OP_LOGICAL_NOT, VT::Object}] = [](const Value& r) { return std::get<Object>(r)->pairs.empty(); };
    unaryOps[{OP_LOGICAL_NOT, VT::Function}] = [](const Value&) { return false; };
    unaryOps[{OP_LOGICAL_NOT, VT::Instance}] = [](const Value&) { return false; };
    unaryOps[{OP_LOGICAL_NOT, VT::Class}] = [](const Value&) { return false; };
    unaryOps[{OP_LOGICAL_NOT, VT::BoundMethod}] = [](const Value&) { return false; };
}

BinaryOpFunc* OperatorDispatcher::find(TokenType op, const Value& left, const Value& right) {
    auto key = BinaryOperationKey{op, getValueType(left), getValueType(right)};
    auto it = binaryOps.find(key);
    return (it != binaryOps.end()) ? &it->second : nullptr;
}

UnaryOpFunc* OperatorDispatcher::find(TokenType op, const Value& right) {
    auto key = UnaryOperationKey{op, getValueType(right)};
    auto it = unaryOps.find(key);
    return (it != unaryOps.end()) ? &it->second : nullptr;
}