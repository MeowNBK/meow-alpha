#pragma once

#include "common/source_file.hpp"
#include <string>
#include <memory>
#include <array>
#include <string_view>
#include <unordered_map>

enum class TokenType {

    KEYWORD_LET,
    KEYWORD_CONST,
    KEYWORD_WHILE,
    KEYWORD_FOR,
    KEYWORD_IF,
    KEYWORD_ELSE,

    KEYWORD_RETURN,
    KEYWORD_BREAK,
    KEYWORD_CONTINUE,
    KEYWORD_FUNCTION,
    KEYWORD_NULL,

    KEYWORD_CLASS,
    KEYWORD_THIS,
    KEYWORD_SUPER,
    KEYWORD_NEW,
    KEYWORD_STATIC,
    KEYWORD_THROW,

    KEYWORD_TRY,
    KEYWORD_CATCH,

    KEYWORD_IMPORT,
    KEYWORD_EXPORT,
    KEYWORD_AS,
    KEYWORD_FROM,

    KEYWORD_SWITCH,
    KEYWORD_CASE,
    KEYWORD_DEFAULT,

    KEYWORD_DO,

    KEYWORD_IN,
    KEYWORD_LOG,

    IDENTIFIER,

    INTEGER,
    REAL,
    BOOLEAN,
    STRING,
    CHAR,

    OP_PLUS,
    OP_MINUS,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_MODULO,

    OP_BIT_AND,
    OP_BIT_OR,
    OP_BIT_XOR,
    OP_BIT_NOT,
    OP_LSHIFT,
    OP_RSHIFT,

    OP_EQ,
    OP_NEQ,
    OP_LT,
    OP_GT,
    OP_LE,
    OP_GE,

    OP_LOGICAL_AND,
    OP_LOGICAL_OR,
    OP_LOGICAL_NOT,

    OP_ASSIGN,
    

    OP_PLUS_ASSIGN,
    OP_MINUS_ASSIGN,
    OP_MULTIPLY_ASSIGN,
    OP_DIVIDE_ASSIGN,
    OP_MODULO_ASSIGN,
    OP_EXPONENT_ASSIGN,

    OP_AND_ASSIGN,
    OP_OR_ASSIGN,
    OP_XOR_ASSIGN,
    OP_NOT_ASSIGN,
    OP_LSHIFT_ASSIGN,
    OP_RSHIFT_ASSIGN,

    OP_INCREMENT,
    OP_DECREMENT,

    OP_ELLIPSIS,
    OP_NULLISH,
    OP_EXPONENT,

    PUNCT_SEMICOLON,
    PUNCT_COLON,
    PUNCT_COMMA,
    PUNCT_LPAREN,
    PUNCT_RPAREN,
    PUNCT_LBRACE,
    PUNCT_RBRACE,
    PUNCT_LBRACKET,
    PUNCT_RBRACKET,
    PUNCT_DOT,
    PUNCT_QUESTION,

    PUNCT_BACKTICK,
    PUNCT_PERCENT_LBRACE,

    UNKNOWN,
    END_OF_FILE,

    _TOTAL_TOKENS,
};

struct Token {
    TokenType type;
    std::string lexeme;
    std::string filename;
    size_t line;
    size_t col;
    SrcFilePtr srcFile;

    Token (
        TokenType type, 
        const std::string& lex, 
        const std::string& path, 
        size_t line, size_t col, 
        SrcFilePtr src
    ): 
        type(type), lexeme(lex), 
        filename(path), 
        line(line), col(col), 
        srcFile(std::move(src)) 
    {}

    inline std::string getLine() const {
        if (srcFile) {
            return srcFile->line(line);
        }
        return {};
    }
};

constexpr std::array<std::string_view, static_cast<size_t>(TokenType::_TOTAL_TOKENS)> tokenTypeNames = {

    "KEYWORD_LET", "KEYWORD_CONST", "KEYWORD_WHILE", "KEYWORD_FOR", "KEYWORD_IF", "KEYWORD_ELSE",
    "KEYWORD_RETURN", "KEYWORD_BREAK", "KEYWORD_CONTINUE", "KEYWORD_FUNCTION", "KEYWORD_NULL",
    "KEYWORD_CLASS", "KEYWORD_THIS", "KEYWORD_SUPER", "KEYWORD_NEW", "KEYWORD_STATIC", "KEYWORD_THROW",
    "KEYWORD_TRY", "KEYWORD_CATCH",
    "KEYWORD_IMPORT", "KEYWORD_EXPORT", "KEYWORD_AS", "KEYWORD_FROM",
    "KEYWORD_SWITCH", "KEYWORD_CASE", "KEYWORD_DEFAULT",
    "KEYWORD_DO", "KEYWORD_IN", "KEYWORD_LOG",

    "IDENTIFIER",

    "INTEGER", "REAL", "BOOLEAN", "STRING", "CHAR",

    "OP_PLUS", "OP_MINUS", "OP_MULTIPLY", "OP_DIVIDE", "OP_MODULO",

    "OP_BIT_AND", "OP_BIT_OR", "OP_BIT_XOR", "OP_BIT_NOT", "OP_LSHIFT", "OP_RSHIFT",

    "OP_EQ", "OP_NEQ", "OP_LT", "OP_GT", "OP_LE", "OP_GE",

    "OP_LOGICAL_AND", "OP_LOGICAL_OR", "OP_LOGICAL_NOT",

    "OP_ASSIGN",

    "OP_PLUS_ASSIGN", "OP_MINUS_ASSIGN", "OP_MULTIPLY_ASSIGN", "OP_DIVIDE_ASSIGN", "OP_MODULO_ASSIGN", "OP_EXPONENT_ASSIGN",

    "OP_AND_ASSIGN", "OP_OR_ASSIGN", "OP_XOR_ASSIGN", "OP_NOT_ASSIGN", "OP_LSHIFT_ASSIGN", "OP_RSHIFT_ASSIGN",

    "OP_INCREMENT", "OP_DECREMENT",
    "OP_ELLIPSIS", "OP_NULLISH", "OP_EXPONENT",

    "PUNCT_SEMICOLON", "PUNCT_COLON", "PUNCT_COMMA", "PUNCT_LPAREN", "PUNCT_RPAREN", "PUNCT_LBRACE", "PUNCT_RBRACE", "PUNCT_LBRACKET", "PUNCT_RBRACKET", "PUNCT_DOT", "PUNCT_QUESTION",

    "PUNCT_BACKTICK", "PUNCT_PERCENT_LBRACE",

    "UNKNOWN", "END_OF_FILE",
};

constexpr std::string_view tokenTypeToString(const TokenType &type) noexcept {
    const size_t index = static_cast<size_t>(type);
    if (index >= tokenTypeNames.size()) {
        return "INVALID";
    }
    return tokenTypeNames[index];
}

inline TokenType stringToTokenType(const std::string_view& tokenStr) {
    static const std::unordered_map<std::string_view, TokenType> tokenMap = {
        {"KEYWORD_LET", TokenType::KEYWORD_LET},
        {"KEYWORD_CONST", TokenType::KEYWORD_CONST},
        {"KEYWORD_WHILE", TokenType::KEYWORD_WHILE},
        {"KEYWORD_FOR", TokenType::KEYWORD_FOR},
        {"KEYWORD_IF", TokenType::KEYWORD_IF},
        {"KEYWORD_ELSE", TokenType::KEYWORD_ELSE},
        {"KEYWORD_RETURN", TokenType::KEYWORD_RETURN},
        {"KEYWORD_BREAK", TokenType::KEYWORD_BREAK},
        {"KEYWORD_CONTINUE", TokenType::KEYWORD_CONTINUE},
        {"KEYWORD_FUNCTION", TokenType::KEYWORD_FUNCTION},
        {"KEYWORD_NULL", TokenType::KEYWORD_NULL},
        {"KEYWORD_CLASS", TokenType::KEYWORD_CLASS},
        {"KEYWORD_THIS", TokenType::KEYWORD_THIS},
        {"KEYWORD_SUPER", TokenType::KEYWORD_SUPER},
        {"KEYWORD_NEW", TokenType::KEYWORD_NEW},
        {"KEYWORD_STATIC", TokenType::KEYWORD_STATIC},
        {"KEYWORD_THROW", TokenType::KEYWORD_THROW},
        {"KEYWORD_TRY", TokenType::KEYWORD_TRY},
        {"KEYWORD_CATCH", TokenType::KEYWORD_CATCH},
        {"KEYWORD_IMPORT", TokenType::KEYWORD_IMPORT},
        {"KEYWORD_EXPORT", TokenType::KEYWORD_EXPORT},
        {"KEYWORD_AS", TokenType::KEYWORD_AS},
        {"KEYWORD_FROM", TokenType::KEYWORD_FROM},
        {"KEYWORD_SWITCH", TokenType::KEYWORD_SWITCH},
        {"KEYWORD_CASE", TokenType::KEYWORD_CASE},
        {"KEYWORD_DEFAULT", TokenType::KEYWORD_DEFAULT},
        {"KEYWORD_DO", TokenType::KEYWORD_DO},
        {"KEYWORD_IN", TokenType::KEYWORD_IN},
        {"KEYWORD_LOG", TokenType::KEYWORD_LOG},
        {"IDENTIFIER", TokenType::IDENTIFIER},
        {"INTEGER", TokenType::INTEGER},
        {"REAL", TokenType::REAL},
        {"BOOLEAN", TokenType::BOOLEAN},
        {"STRING", TokenType::STRING},
        {"CHAR", TokenType::CHAR},
        {"OP_PLUS", TokenType::OP_PLUS},
        {"OP_MINUS", TokenType::OP_MINUS},
        {"OP_MULTIPLY", TokenType::OP_MULTIPLY},
        {"OP_DIVIDE", TokenType::OP_DIVIDE},
        {"OP_MODULO", TokenType::OP_MODULO},
        {"OP_EXPONENT_ASSIGN", TokenType::OP_EXPONENT_ASSIGN},
        {"OP_BIT_AND", TokenType::OP_BIT_AND},
        {"OP_BIT_OR", TokenType::OP_BIT_OR},
        {"OP_BIT_XOR", TokenType::OP_BIT_XOR},
        {"OP_BIT_NOT", TokenType::OP_BIT_NOT},
        {"OP_LSHIFT", TokenType::OP_LSHIFT},
        {"OP_RSHIFT", TokenType::OP_RSHIFT},
        {"OP_EQ", TokenType::OP_EQ},
        {"OP_NEQ", TokenType::OP_NEQ},
        {"OP_LT", TokenType::OP_LT},
        {"OP_GT", TokenType::OP_GT},
        {"OP_LE", TokenType::OP_LE},
        {"OP_GE", TokenType::OP_GE},
        {"OP_LOGICAL_AND", TokenType::OP_LOGICAL_AND},
        {"OP_LOGICAL_OR", TokenType::OP_LOGICAL_OR},
        {"OP_LOGICAL_NOT", TokenType::OP_LOGICAL_NOT},
        {"OP_ASSIGN", TokenType::OP_ASSIGN},
        {"OP_PLUS_ASSIGN", TokenType::OP_PLUS_ASSIGN},
        {"OP_MINUS_ASSIGN", TokenType::OP_MINUS_ASSIGN},
        {"OP_MULTIPLY_ASSIGN", TokenType::OP_MULTIPLY_ASSIGN},
        {"OP_DIVIDE_ASSIGN", TokenType::OP_DIVIDE_ASSIGN},
        {"OP_MODULO_ASSIGN", TokenType::OP_MODULO_ASSIGN},
        {"OP_AND_ASSIGN", TokenType::OP_AND_ASSIGN},
        {"OP_OR_ASSIGN", TokenType::OP_OR_ASSIGN},
        {"OP_XOR_ASSIGN", TokenType::OP_XOR_ASSIGN},
        {"OP_NOT_ASSIGN", TokenType::OP_NOT_ASSIGN},
        {"OP_LSHIFT_ASSIGN", TokenType::OP_LSHIFT_ASSIGN},
        {"OP_RSHIFT_ASSIGN", TokenType::OP_RSHIFT_ASSIGN},
        {"OP_INCREMENT", TokenType::OP_INCREMENT},
        {"OP_DECREMENT", TokenType::OP_DECREMENT},
        {"OP_ELLIPSIS", TokenType::OP_ELLIPSIS},
        {"OP_NULLISH", TokenType::OP_NULLISH},
        {"OP_EXPONENT", TokenType::OP_EXPONENT},
        {"PUNCT_SEMICOLON", TokenType::PUNCT_SEMICOLON},
        {"PUNCT_COLON", TokenType::PUNCT_COLON},
        {"PUNCT_COMMA", TokenType::PUNCT_COMMA},
        {"PUNCT_LPAREN", TokenType::PUNCT_LPAREN},
        {"PUNCT_RPAREN", TokenType::PUNCT_RPAREN},
        {"PUNCT_LBRACE", TokenType::PUNCT_LBRACE},
        {"PUNCT_RBRACE", TokenType::PUNCT_RBRACE},
        {"PUNCT_LBRACKET", TokenType::PUNCT_LBRACKET},
        {"PUNCT_RBRACKET", TokenType::PUNCT_RBRACKET},
        {"PUNCT_DOT", TokenType::PUNCT_DOT},
        {"PUNCT_QUESTION", TokenType::PUNCT_QUESTION},
        {"PUNCT_BACKTICK", TokenType::PUNCT_BACKTICK},
        {"PUNCT_PERCENT_LBRACE", TokenType::PUNCT_PERCENT_LBRACE},
        {"UNKNOWN", TokenType::UNKNOWN},
        {"END_OF_FILE", TokenType::END_OF_FILE},
    };

    auto it = tokenMap.find(tokenStr);
    if (it != tokenMap.end()) {
        return it->second;
    }
    return TokenType::UNKNOWN;
}