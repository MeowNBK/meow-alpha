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

    const std::string& src_, filename_;
    size_t pos_;
    unsigned char curr_char_;
    size_t line_, col_;
    size_t start_line_, start_col_;
    bool is_in_template_mode_ = false;
    bool is_in_expression_ = false;

    void advance() noexcept;
    unsigned char peek() const noexcept;

    void skip_whitespace() noexcept;
    void skip_line_comment() noexcept;
    void skip_block_comment() noexcept;

    inline Token make_token(TokenType type, const std::string &lex) noexcept;

    Token next_token();

    Token identifier();
    Token number();
    Token string_literal(unsigned char delimitier);
    Token punctuator();
    Token template_string();
    Token raw_string(unsigned char delimitier);
};
