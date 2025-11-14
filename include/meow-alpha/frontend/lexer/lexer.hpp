#pragma once

#include "common/token.hpp"
#include "common/source_file.hpp"
#include <vector>

class Lexer {
public:
    Lexer(SrcFilePtr sourceFile);

    std::vector<Token> tokenize();

private:
    SrcFilePtr srcFile; 

    const std::string& src;
    const std::string& filename;
    size_t pos;
    char currChar;
    size_t line, col;
    bool isInTemplateMode = false;
    bool isInExpression = false;
    bool pendingTemplateExpr = false;

    void advance();
    char peek() const;

    void skipWhiteSpace();
    void skipLineComment();
    void skipBlockComment();

    inline Token makeToken(TokenType type, const std::string &lex, size_t startLine, size_t startCol);

    Token nextToken();

    Token identifier(size_t startLine, size_t startCol);
    Token number(size_t startLine, size_t startCol);
    Token stringLiteral(char delimitier, size_t startLine, size_t startCol);
    Token punctuator(size_t startLine, size_t startCol);
    Token templateStringLiteral(size_t startLine, size_t startCol);
    Token rawStringLiteral(char delimitier, size_t startLine, size_t startCol);
};
