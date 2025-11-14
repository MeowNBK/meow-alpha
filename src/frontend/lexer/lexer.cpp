#include "lexer/lexer.hpp"
#include "lexer/keywords.hpp"
#include "lexer/symbols.hpp"
#include <cctype>
#include <unordered_map>
#include <iostream>

using enum TokenType;

Lexer::Lexer(SrcFilePtr sourceFile): 
    srcFile(std::move(sourceFile)), 
    src(srcFile->getBuffer()), 
    filename(srcFile->name()),
    pos(0), 
    currChar(srcFile->getBuffer().empty() ? '\0' : srcFile->getBuffer()[0]), 
    line(1), col(1) 
{}

void Lexer::advance() {
    if (currChar == '\n') {
        ++line;
        col = 1;
    } else {
        ++col;
    }
    ++pos;
    currChar = pos < src.size() ? src[pos] : '\0';
}

char Lexer::peek() const{
    size_t next = pos + 1;
    return next < src.size() ? src[next] : '\0';
}

void Lexer::skipWhiteSpace() {
    while (std::isspace(currChar)) {
        advance();
    }
}

void Lexer::skipLineComment() {
    advance(); advance();
    while (currChar != '\n' && currChar != '\0') {
        advance();
    }
}

void Lexer::skipBlockComment() {
    advance(); advance();

    while (!(currChar == '*' && peek() == '/') && currChar != '\0') {
        advance();
    }

    if (currChar != '\0' && peek() != '\0') {
        advance(); advance();
    }
}

inline Token Lexer::makeToken(TokenType type, const std::string &lex, size_t startLine, size_t startCol) {
    return {type, lex, filename, startLine, startCol, srcFile};
}

Token Lexer::identifier(size_t startLine, size_t startCol) {
    std::string lex = std::string(1, currChar);
    advance();
    while (std::isalnum(static_cast<unsigned char>(currChar)) || currChar == '_') {
        lex += currChar;
        advance();
    }
    
    if (auto it = keywords.find(lex); it != keywords.end()) {
        return makeToken(it->second, lex, startLine, startCol);
    }

    return makeToken(IDENTIFIER, lex, startLine, startCol);
}

Token Lexer::number(size_t startLine, size_t startCol) { 
    std::string lex;
    bool isReal = false;

    if (currChar == '0') {
        lex += currChar;
        advance();

        if (currChar == 'x' || currChar == 'X') {
            lex += currChar;
            advance();
            while (std::isxdigit(static_cast<unsigned char>(currChar)) || currChar == '_') {
                if (currChar != '_') lex += currChar;
                advance();
            }
            return makeToken(INTEGER, lex, startLine, startCol);
        }
        else if (currChar == 'b' || currChar == 'B') {
            lex += currChar;
            advance();
            while (currChar == '0' || currChar == '1' || currChar == '_') {
                if (currChar != '_') lex += currChar;
                advance();
            }
            return makeToken(INTEGER, lex, startLine, startCol);
        }
        else if (currChar == 'o' || currChar == 'O') {
            lex += currChar;
            advance();
            while ((currChar >= '0' && currChar <= '7') || currChar == '_') {
                if (currChar != '_') lex += currChar;
                advance();
            }
            return makeToken(INTEGER, lex, startLine, startCol);
        }
    }

    while (std::isdigit(static_cast<unsigned char>(currChar)) || currChar == '_' || currChar == '.') {
        if (currChar == '.') {
            if (isReal) break;
            isReal = true;
            if (!std::isdigit(peek())) break;
        }

        if (currChar != '_') lex += currChar;
        advance();
    }

    if (currChar == 'e' || currChar == 'E') {
        isReal = true;
        lex += currChar;
        advance();
        if (currChar == '+' || currChar == '-') {
            lex += currChar;
            advance();
        }
        while (std::isdigit(static_cast<unsigned char>(currChar)) || currChar == '_') {
            if (currChar != '_') lex += currChar;
            advance();
        }
    }

    return makeToken(isReal ? REAL : INTEGER, lex, startLine, startCol);
}

Token Lexer::stringLiteral(char delimiter, size_t startLine, size_t startCol) {
    advance();

    std::string lex = "";

    while (currChar != delimiter && currChar != '\0') {
        if (currChar == '\\') {
            advance();
            switch (currChar) {
                case '\\': lex += '\\'; break;
                case '"': lex += (delimiter == '"') ? "\"" : "\\\""; break;
                case '\'': lex += (delimiter == '\'') ? "'" : "\\'"; break;
                case 'n': lex += '\n'; break;
                case 't': lex += '\t'; break;
                case 'r': lex += '\r'; break;
                case '0': lex += '\0'; break;
                default:
                    lex += '\\';
                    lex += currChar;
            }
        } else {
            lex += currChar;
        }

        advance();
    }

    if (currChar == delimiter) {
        advance();
    }

    return makeToken(STRING, lex, startLine, startCol);
}

Token Lexer::punctuator(size_t startLine, size_t startCol) {
    for (int len = 3; len >= 1; --len) {
        std::string lex = src.substr(pos, len);
        auto it = symbols.find(lex);

        if (it != symbols.end()) {
            for (int i = 0; i < len; ++i) {
                advance();
            }
            return makeToken(it->second, lex, startLine, startCol);
        }
    }

    return makeToken(UNKNOWN, "", startLine, startCol);
}

Token Lexer::templateStringLiteral(size_t startLine, size_t startCol) {
    std::string lex = "";
    while (currChar != '`' && currChar != '\0' && !(currChar == '%' && peek() == '{')) {
        if (currChar == '\\') {
            advance();
            switch (currChar) {
                case '\\': lex += '\\'; break;
                case '`': lex += '`'; break;
                case 'n': lex += '\n'; break;
                case 't': lex += '\t'; break;
                case 'r': lex += '\r'; break;
                case '0': lex += '\0'; break;
                default:
                    lex += '\\';
                    lex += currChar;
            }
        } else {
            lex += currChar;
        }

        advance();
    }
    return makeToken(STRING, lex, startLine, startCol);
}

Token Lexer::rawStringLiteral(char delimiter, size_t startLine, size_t startCol) {
    std::string lex = "";
    advance();
    while (currChar != delimiter && currChar != '\0') {
        lex += currChar;
        advance();
    }
    if (currChar == delimiter) {
        advance();
    }
    return makeToken(STRING, lex, startLine, startCol);
}

Token Lexer::nextToken() {
    size_t startLine, startCol;

    auto createToken = [&startLine, &startCol, this](TokenType type, const std::string &lexeme) {
        return makeToken(type, lexeme, startLine, startCol);
    };

    if (!isInTemplateMode) {
        skipWhiteSpace();
    }
    
    startLine = line; 
    startCol = col;

        if (isInExpression && currChar == '}') {
            isInTemplateMode = true;
            isInExpression = false;
            Token token = createToken(PUNCT_RBRACE, "}");
            advance();
            return token;
        }

        if (isInTemplateMode) {
            if (currChar == '`') {
                isInTemplateMode = false;
                Token token = createToken(PUNCT_BACKTICK, "`");
                advance();
                return token;
            } else if (currChar == '%' && peek() == '{') {
                isInTemplateMode = false;
                isInExpression = true;
                advance(); advance();
                return createToken(PUNCT_PERCENT_LBRACE, "%{");
            }

            return templateStringLiteral(startLine, startCol);
        } else {
        if (currChar == '\0') {
            return createToken(END_OF_FILE, "");
        } else if ((currChar == 'r' || currChar == 'R') && (peek() == '"' || peek() == '\'')) {
            advance();
            return rawStringLiteral(currChar, startLine, startCol);
        } else if (std::isalpha(static_cast<unsigned char>(currChar)) || currChar == '_') {
            return identifier(startLine, startCol);
        } else if (std::isdigit(static_cast<unsigned char>(currChar))) {
            return number(startLine, startCol);
        } else {
            switch (currChar) {
                case '`': {
                    isInTemplateMode = true;
                    Token token = createToken(PUNCT_BACKTICK, "`");
                    advance();
                    return token;
                }
                case '"':
                    return stringLiteral('"', startLine, startCol);
                case '\'':
                    return stringLiteral('\'', startLine, startCol);
                case '/':
                    if (peek() == '/') {
                        skipLineComment();
                        return nextToken();
                    } else if (peek() == '*') {
                        skipBlockComment();
                        return nextToken();
                    }
                    return punctuator(startLine, startCol);
                default:
                    return punctuator(startLine, startCol);              
            }
        }
    }
}

std::vector<Token> Lexer::tokenize() {
    isInTemplateMode = false;
    isInExpression = false;
    pendingTemplateExpr = false;
    std::vector<Token> tokens;

    while (true) {
        Token token = nextToken();

        tokens.push_back(token);

        if (token.type == END_OF_FILE) break;
    }

    return tokens;
}