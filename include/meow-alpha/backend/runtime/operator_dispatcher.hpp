#pragma once

#include "runtime/value.hpp" 
#include "common/token.hpp" 
#include <map>
#include <functional>
#include <string>

enum class ValueType {
    Null, Int, Real, Bool, String, Array, Object, Function, Instance, Class, BoundMethod
};

ValueType getValueType(const Value& value);
std::string getBinaryOperatorMethodName(TokenType opType);
std::string getUnaryOperatorMethodName(TokenType opType);

struct BinaryOperationKey {
    TokenType op;
    ValueType left;
    ValueType right;
    
    bool operator<(const BinaryOperationKey& other) const {
        if (op != other.op) return op < other.op;
        if (left != other.left) return left < other.left;
        return right < other.right;
    }
};

struct UnaryOperationKey {
    TokenType op;
    ValueType right;
    
    bool operator<(const UnaryOperationKey& other) const {
        if (op != other.op) return op < other.op;
        return right < other.right;
    }
};

using BinaryOpFunc = std::function<Value(const Value&, const Value&)>;
using UnaryOpFunc = std::function<Value(const Value&)>;

class OperatorDispatcher {
public:
    std::map<BinaryOperationKey, BinaryOpFunc> binaryOps;
    std::map<UnaryOperationKey, UnaryOpFunc> unaryOps;

    OperatorDispatcher();

    BinaryOpFunc* find(TokenType op, const Value& left, const Value& right);
    UnaryOpFunc* find(TokenType op, const Value& right);
};