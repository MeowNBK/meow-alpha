#pragma once

#include "common/token.hpp"
#include <unordered_map>
#include <string>

const std::unordered_map<std::string, TokenType> symbols = {
    {"+",       TokenType::OP_PLUS},
    {"-",       TokenType::OP_MINUS},
    {"*",       TokenType::OP_MULTIPLY},
    {"/",       TokenType::OP_DIVIDE},
    {"%",       TokenType::OP_MODULO},

    {"&",       TokenType::OP_BIT_AND},
    {"|",       TokenType::OP_BIT_OR},
    {"^",       TokenType::OP_BIT_XOR},
    {"~",       TokenType::OP_BIT_NOT},
    {"<<",      TokenType::OP_LSHIFT},
    {">>",      TokenType::OP_RSHIFT},

    {"==",      TokenType::OP_EQ},
    {"!=",      TokenType::OP_NEQ},
    {"<",       TokenType::OP_LT},
    {">",       TokenType::OP_GT},
    {"<=",      TokenType::OP_LE},
    {">=",      TokenType::OP_GE},

    {"&&",      TokenType::OP_LOGICAL_AND},
    {"||",      TokenType::OP_LOGICAL_OR},
    {"!",       TokenType::OP_LOGICAL_NOT},

    {"=",       TokenType::OP_ASSIGN},

    {"+=",      TokenType::OP_PLUS_ASSIGN},
    {"-=",      TokenType::OP_MINUS_ASSIGN},
    {"*=",      TokenType::OP_MULTIPLY_ASSIGN},
    {"/=",      TokenType::OP_DIVIDE_ASSIGN},
    {"%=",      TokenType::OP_MODULO_ASSIGN},
    {"**=",     TokenType::OP_EXPONENT_ASSIGN},

    {"&=",      TokenType::OP_AND_ASSIGN},
    {"|=",      TokenType::OP_OR_ASSIGN},
    {"^=",      TokenType::OP_XOR_ASSIGN},
    {"~=",      TokenType::OP_NOT_ASSIGN},
    {"<<=",     TokenType::OP_LSHIFT_ASSIGN},
    {">>=",     TokenType::OP_RSHIFT_ASSIGN},

    {"++",      TokenType::OP_INCREMENT},
    {"--",      TokenType::OP_DECREMENT},

    {"...",     TokenType::OP_ELLIPSIS},
    {"??",      TokenType::OP_NULLISH},
    {"**",      TokenType::OP_EXPONENT},

    {";",       TokenType::PUNCT_SEMICOLON},
    {":",       TokenType::PUNCT_COLON},
    {",",       TokenType::PUNCT_COMMA},
    {"(",       TokenType::PUNCT_LPAREN},
    {")",       TokenType::PUNCT_RPAREN},
    {"{",       TokenType::PUNCT_LBRACE},
    {"}",       TokenType::PUNCT_RBRACE},
    {"[",       TokenType::PUNCT_LBRACKET},
    {"]",       TokenType::PUNCT_RBRACKET},
    {".",       TokenType::PUNCT_DOT},
    {"?",       TokenType::PUNCT_QUESTION},

    {"`",       TokenType::PUNCT_BACKTICK},
    {"%{",      TokenType::PUNCT_QUESTION},
};