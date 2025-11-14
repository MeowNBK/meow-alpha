#pragma once

#include "common/ast.hpp"
#include <functional>

class Parser;

enum class Precedence {
    NONE,
    ASSIGN, 
    NULLISH,
    TERNARY,
    LOGICAL_OR,
    LOGICAL_AND,
    BITWISE_OR,
    BITWISE_XOR,
    BITWISE_AND,
    EQUALITY,
    COMPARISON,
    BITWISE_SHIFT,
    SUM,
    PRODUCT,
    EXPONENT,
    UNARY,
    CALL,
    INDEX,
    PRIMARY,
};

using prefixFn = std::function<ExprPtr(Parser*)>;
using infixFn = std::function<ExprPtr(Parser*, ExprPtr)>;

struct ParseRule {
    prefixFn prefix;
    infixFn infix;
    Precedence precedence;
};