#pragma once

#include "common/token.hpp"

#include <unordered_map>
#include <string>

const std::unordered_map<std::string, TokenType> keywords = {
    {"let",             TokenType::KEYWORD_LET},
    {"const",           TokenType::KEYWORD_CONST},
    {"while",           TokenType::KEYWORD_WHILE},
    {"for",             TokenType::KEYWORD_FOR},
    {"if",              TokenType::KEYWORD_IF},
    {"else",            TokenType::KEYWORD_ELSE},
    {"return",          TokenType::KEYWORD_RETURN},
    {"break",           TokenType::KEYWORD_BREAK},
    {"continue",        TokenType::KEYWORD_CONTINUE},
    {"function",        TokenType::KEYWORD_FUNCTION},
    {"fn",              TokenType::KEYWORD_FUNCTION},
    {"null",            TokenType::KEYWORD_NULL},

    {"class",           TokenType::KEYWORD_CLASS},
    {"this",            TokenType::KEYWORD_THIS},
    {"super",           TokenType::KEYWORD_SUPER},
    {"new",             TokenType::KEYWORD_NEW},
    {"static",          TokenType::KEYWORD_STATIC},
    {"throw",           TokenType::KEYWORD_THROW},
    {"import",          TokenType::KEYWORD_IMPORT},
    {"export",          TokenType::KEYWORD_EXPORT},
    {"as",              TokenType::KEYWORD_AS},
    {"from",            TokenType::KEYWORD_FROM},
    {"try",             TokenType::KEYWORD_TRY},
    {"catch",           TokenType::KEYWORD_CATCH},

    {"switch",          TokenType::KEYWORD_SWITCH},
    {"case",            TokenType::KEYWORD_CASE},
    {"default",         TokenType::KEYWORD_DEFAULT},
    
    {"do",              TokenType::KEYWORD_DO},
    {"in",              TokenType::KEYWORD_IN},

    {"log",             TokenType::KEYWORD_LOG},

    {"true",            TokenType::BOOLEAN},
    {"false",           TokenType::BOOLEAN},
};