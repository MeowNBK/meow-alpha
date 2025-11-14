#include "lexer/lexer.hpp"
#include "lexer/keywords.hpp"
#include "lexer/symbols.hpp"
#include <cctype>
#include <unordered_map>
#include <iostream>

using enum TokenType;

Lexer::Lexer(SrcFilePtr sourceFile): 
    srcFile(std::move(sourceFile)), 
    src_(srcFile->getBuffer()), 
    filename_(srcFile->name()),
    pos_(0), 
    curr_char_(srcFile->getBuffer().empty() ? '\0' : srcFile->getBuffer()[0]), 
    line_(1), col_(1),
    start_line_(1), start_col_(1)
{}

void Lexer::advance() noexcept {
    if (curr_char_ == '\n') {
        ++line_;
        col_ = 1;
    } else {
        ++col_;
    }
    ++pos_;
    curr_char_ = pos_ < src_.size() ? src_[pos_] : '\0';
}

unsigned char Lexer::peek() const noexcept {
    size_t next = pos_ + 1;
    return next < src_.size() ? src_[next] : '\0';
}

void Lexer::skip_whitespace() noexcept {
    while (std::isspace(curr_char_)) {
        advance();
    }
}

void Lexer::skip_line_comment() noexcept {
    advance(); advance();
    while (curr_char_ != '\n' && curr_char_ != '\0') {
        advance();
    }
}

void Lexer::skip_block_comment() noexcept {
    advance(); advance();

    while (!(curr_char_ == '*' && peek() == '/') && curr_char_ != '\0') {
        advance();
    }

    if (curr_char_ != '\0' && peek() != '\0') {
        advance(); advance();
    }
}

inline Token Lexer::make_token(TokenType type, const std::string &lex) noexcept {
    return { type, lex, filename_, start_line_, start_col_, srcFile };
}

Token Lexer::identifier() {
    std::string lex = std::string(1, curr_char_);
    advance();
    while (std::isalnum(curr_char_) || curr_char_ == '_') {
        lex += curr_char_;
        advance();
    }
    
    if (auto it = keywords.find(lex); it != keywords.end()) {
        return make_token(it->second, lex);
    }

    return make_token(IDENTIFIER, lex);
}

Token Lexer::number() { 
    std::string lex;
    bool isReal = false;

    if (curr_char_ == '0') {
        lex += curr_char_;
        advance();

        if (curr_char_ == 'x' || curr_char_ == 'X') {
            lex += curr_char_;
            advance();
            while (std::isxdigit(curr_char_) || curr_char_ == '_') {
                if (curr_char_ != '_') lex += curr_char_;
                advance();
            }
            return make_token(INTEGER, lex);
        }
        else if (curr_char_ == 'b' || curr_char_ == 'B') {
            lex += curr_char_;
            advance();
            while (curr_char_ == '0' || curr_char_ == '1' || curr_char_ == '_') {
                if (curr_char_ != '_') lex += curr_char_;
                advance();
            }
            return make_token(INTEGER, lex);
        }
        else if (curr_char_ == 'o' || curr_char_ == 'O') {
            lex += curr_char_;
            advance();
            while ((curr_char_ >= '0' && curr_char_ <= '7') || curr_char_ == '_') {
                if (curr_char_ != '_') lex += curr_char_;
                advance();
            }
            return make_token(INTEGER, lex);
        }
    }

    while (std::isdigit(curr_char_) || curr_char_ == '_' || curr_char_ == '.') {
        if (curr_char_ == '.') {
            if (isReal) break;
            isReal = true;
            if (!std::isdigit(peek())) break;
        }

        if (curr_char_ != '_') lex += curr_char_;
        advance();
    }

    if (curr_char_ == 'e' || curr_char_ == 'E') {
        isReal = true;
        lex += curr_char_;
        advance();
        if (curr_char_ == '+' || curr_char_ == '-') {
            lex += curr_char_;
            advance();
        }
        while (std::isdigit(curr_char_) || curr_char_ == '_') {
            if (curr_char_ != '_') lex += curr_char_;
            advance();
        }
    }

    return make_token(isReal ? REAL : INTEGER, lex);
}

Token Lexer::string_literal(unsigned char delimiter) {
    advance();

    std::string lex = "";

    while (curr_char_ != delimiter && curr_char_ != '\0') {
        if (curr_char_ == '\\') {
            advance();
            switch (curr_char_) {
                case '\\': lex += '\\'; break;
                case '"': lex += (delimiter == '"') ? "\"" : "\\\""; break;
                case '\'': lex += (delimiter == '\'') ? "'" : "\\'"; break;
                case 'n': lex += '\n'; break;
                case 't': lex += '\t'; break;
                case 'r': lex += '\r'; break;
                case '0': lex += '\0'; break;
                default:
                    lex += '\\';
                    lex += curr_char_;
            }
        } else {
            lex += curr_char_;
        }

        advance();
    }

    if (curr_char_ == delimiter) {
        advance();
    }

    return make_token(STRING, lex);
}

Token Lexer::punctuator() {
    for (int len = 3; len >= 1; --len) {
        std::string lex = src_.substr(pos_, len);
        auto it = symbols.find(lex);

        if (it != symbols.end()) {
            for (int i = 0; i < len; ++i) {
                advance();
            }
            return make_token(it->second, lex);
        }
    }

    return make_token(UNKNOWN, "");
}

Token Lexer::template_string() {
    std::string lex = "";
    while (curr_char_ != '`' && curr_char_ != '\0' && !(curr_char_ == '%' && peek() == '{')) {
        if (curr_char_ == '\\') {
            advance();
            switch (curr_char_) {
                case '\\': lex += '\\'; break;
                case '`': lex += '`'; break;
                case 'n': lex += '\n'; break;
                case 't': lex += '\t'; break;
                case 'r': lex += '\r'; break;
                case '0': lex += '\0'; break;
                default:
                    lex += '\\';
                    lex += curr_char_;
            }
        } else {
            lex += curr_char_;
        }

        advance();
    }
    return make_token(STRING, lex);
}

Token Lexer::raw_string(unsigned char delimiter) {
    std::string lex = "";
    advance();
    while (curr_char_ != delimiter && curr_char_ != '\0') {
        lex += curr_char_;
        advance();
    }
    if (curr_char_ == delimiter) {
        advance();
    }
    return make_token(STRING, lex);
}

Token Lexer::next_token() {
    if (!is_in_template_mode_) skip_whitespace();
    start_line_ = line_; 
    start_col_ = col_;
    if (is_in_expression_ && curr_char_ == '}') {
        is_in_template_mode_ = true;
        is_in_expression_ = false;
        Token token = make_token(PUNCT_RBRACE, "}");
        advance();
        return token;
    }

    if (is_in_template_mode_) {
        if (curr_char_ == '`') {
            is_in_template_mode_ = false;
            Token token = make_token(PUNCT_BACKTICK, "`");
            advance();
            return token;
        } else if (curr_char_ == '%' && peek() == '{') {
            is_in_template_mode_ = false;
            is_in_expression_ = true;
            advance(); advance();
            return make_token(PUNCT_PERCENT_LBRACE, "%{");
        }

        return template_string();
    } else {
        if (curr_char_ == '\0') {
            return make_token(END_OF_FILE, "");
        } else if ((curr_char_ == 'r' || curr_char_ == 'R') && (peek() == '"' || peek() == '\'')) {
            advance();
            return raw_string(curr_char_);
        } else if (std::isalpha(curr_char_) || curr_char_ == '_') {
            return identifier();
        } else if (std::isdigit(curr_char_)) {
            return number();
        } else {
            switch (curr_char_) {
                case '`': {
                    is_in_template_mode_ = true;
                    Token token = make_token(PUNCT_BACKTICK, "`");
                    advance();
                    return token;
                }
                case '"':
                    return string_literal('"');
                case '\'':
                    return string_literal('\'');
                case '/':
                    if (peek() == '/') {
                        skip_line_comment();
                        return next_token();
                    } else if (peek() == '*') {
                        skip_block_comment();
                        return next_token();
                    }
                    return punctuator();
                default:
                    return punctuator();              
            }
        }
    }
}

std::vector<Token> Lexer::tokenize() {
    is_in_template_mode_ = false;
    is_in_expression_ = false;
    std::vector<Token> tokens;

    while (true) {
        Token token = next_token();

        tokens.push_back(token);

        if (token.type == END_OF_FILE) break;
    }

    return tokens;
}