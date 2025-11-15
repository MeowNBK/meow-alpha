#include "parser/parser.hpp"
#include <iostream>

using enum TokenType;

Parser::Parser(const std::vector<Token> &tokens): tokens(tokens), current(0) {
    initRules();
}

std::unique_ptr<Program> Parser::parseProgram() {
    std::unique_ptr<Program> program = std::make_unique<Program>();
    while (!isAtEnd()) {
        try {
            auto decl = declaration();
            if (decl) {
                program->body.push_back(std::move(decl));
            }
        } catch (Diagnostic& e) {
            std::cerr << e.str() << "\n";
            synchronize();
        }
    }
    return program;
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::END_OF_FILE;
}

const Token& Parser::peek() const {
    return tokens[current];
}

const Token& Parser::previous() const {
    return tokens[current - 1];
}

const Token& Parser::next() const {
    size_t next = current + 1;
    if (!isAtEnd()) return tokens[next];
    return peek();
}

const Token& Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (auto type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

const Token& Parser::consume(TokenType type, const std::string &errMsg) {
    if (check(type)) return advance();

    throw Diagnostic::ParseErr(errMsg, peek());
}

void Parser::synchronize() {
    advance();
    while (!isAtEnd()) {
        if (previous().type == TokenType::PUNCT_SEMICOLON) return;
        switch (peek().type) {
            case TokenType::KEYWORD_IF: 
            case TokenType::KEYWORD_FOR:
            case TokenType::KEYWORD_WHILE: 
            case TokenType::KEYWORD_RETURN:
            case TokenType::KEYWORD_LET:
            case TokenType::KEYWORD_CONST:
            case TokenType::KEYWORD_FUNCTION:
                return;
            default: break;
        }
        advance();
    }
}

void Parser::initRules() {
    using enum TokenType;
    using enum Precedence;
    const ParseRule defaultRule = {nullptr, nullptr, NONE};

    for (int i = 0; i < static_cast<int>(_TOTAL_TOKENS); ++i) {
        rules[(TokenType)i] = defaultRule;
    }

    rules[INTEGER]        = { &Parser::literal,   nullptr,         PRIMARY };
    rules[REAL]           = { &Parser::literal,   nullptr,         PRIMARY };
    rules[STRING]         = { &Parser::literal,   nullptr,         PRIMARY };
    rules[BOOLEAN]        = { &Parser::literal,   nullptr,         PRIMARY };
    rules[KEYWORD_NULL]   = { &Parser::literal,   nullptr,         PRIMARY };
    rules[PUNCT_LBRACE]   = { &Parser::objectLiteral,   nullptr,         PRIMARY };
    rules[PUNCT_BACKTICK]   = { &Parser::templateLiteral,   nullptr,         PRIMARY };

    rules[IDENTIFIER]     = { &Parser::identifier,nullptr,         PRIMARY };

    rules[KEYWORD_FUNCTION] = { &Parser::functionLiteral, nullptr, PRIMARY };

    rules[PUNCT_LPAREN]   = { &Parser::grouping,  &Parser::call,   CALL };
    rules[OP_MINUS]       = { &Parser::unary,     &Parser::binary, SUM };
    rules[OP_LOGICAL_NOT] = { &Parser::unary,     nullptr,         UNARY };
    rules[OP_LOGICAL_OR]  = { nullptr,            &Parser::binary, LOGICAL_OR };
    rules[OP_LOGICAL_AND] = { nullptr,            &Parser::binary, LOGICAL_AND };

    rules[OP_PLUS]         = { nullptr,          &Parser::binary, SUM };
    rules[OP_MULTIPLY]     = { nullptr,          &Parser::binary, PRODUCT };
    rules[OP_DIVIDE]       = { nullptr,          &Parser::binary, PRODUCT };
    rules[OP_MODULO]       = { nullptr,          &Parser::binary, PRODUCT };
    rules[OP_EXPONENT]       = { nullptr,          &Parser::binary, EXPONENT };
    rules[OP_NULLISH]       = { nullptr,          &Parser::binary, NULLISH };

    rules[OP_BIT_AND]        = { nullptr, &Parser::binary, BITWISE_AND };
    rules[OP_BIT_OR]         = { nullptr, &Parser::binary, BITWISE_OR };
    rules[OP_BIT_XOR]        = { nullptr, &Parser::binary, BITWISE_XOR };
    rules[OP_LSHIFT]         = { nullptr, &Parser::binary, BITWISE_SHIFT };
    rules[OP_RSHIFT]         = { nullptr, &Parser::binary, BITWISE_SHIFT };
    rules[OP_BIT_NOT]        = { &Parser::unary,  nullptr, UNARY };

    rules[OP_PLUS_ASSIGN]     =   { nullptr, &Parser::assignment, ASSIGN };
    rules[OP_MINUS_ASSIGN]    =   { nullptr, &Parser::assignment, ASSIGN };
    rules[OP_MULTIPLY_ASSIGN] =   { nullptr, &Parser::assignment, ASSIGN };
    rules[OP_DIVIDE_ASSIGN]   =   { nullptr, &Parser::assignment, ASSIGN };
    rules[OP_MODULO_ASSIGN]   =   { nullptr, &Parser::assignment, ASSIGN };
    rules[OP_MODULO_ASSIGN]   =   { nullptr, &Parser::assignment, ASSIGN };
    rules[OP_EXPONENT_ASSIGN]   =   { nullptr, &Parser::assignment, ASSIGN };

    rules[OP_AND_ASSIGN]      =   { nullptr, &Parser::assignment, ASSIGN };
    rules[OP_OR_ASSIGN]       =   { nullptr, &Parser::assignment, ASSIGN };
    rules[OP_XOR_ASSIGN]      =   { nullptr, &Parser::assignment, ASSIGN };
    rules[OP_LSHIFT_ASSIGN]   =   { nullptr, &Parser::assignment, ASSIGN };
    rules[OP_RSHIFT_ASSIGN]   =   { nullptr, &Parser::assignment, ASSIGN };

    rules[OP_EQ]           = { nullptr,          &Parser::binary, COMPARISON };
    rules[OP_NEQ]          = { nullptr,          &Parser::binary, COMPARISON };
    rules[OP_LT]           = { nullptr,          &Parser::binary, COMPARISON };
    rules[OP_LE]           = { nullptr,          &Parser::binary, COMPARISON };
    rules[OP_GT]           = { nullptr,          &Parser::binary, COMPARISON };
    rules[OP_GE]           = { nullptr,          &Parser::binary, COMPARISON };

    rules[OP_ASSIGN]       = { nullptr,          &Parser::assignment, ASSIGN };

    rules[OP_INCREMENT]    = { &Parser::prefixUpdate,   &Parser::postfixUpdate, UNARY };
    rules[OP_DECREMENT]    = { &Parser::prefixUpdate,   &Parser::postfixUpdate, UNARY };

    rules[OP_DECREMENT]    = { &Parser::prefixUpdate,   &Parser::postfixUpdate, UNARY };
    rules[OP_DECREMENT]    = { &Parser::prefixUpdate,   &Parser::postfixUpdate, UNARY };
    rules[OP_DECREMENT]    = { &Parser::prefixUpdate,   &Parser::postfixUpdate, UNARY };
    
    rules[PUNCT_LBRACKET]  = { &Parser::arrayLiteral, &Parser::index,      INDEX };
    rules[OP_ELLIPSIS]     = { &Parser::spreadExpr, nullptr,      PRIMARY };
    rules[PUNCT_DOT]       = { nullptr,          &Parser::access,     CALL};

    rules[KEYWORD_THIS]    = { &Parser::thisExpr, nullptr, PRIMARY};
    rules[KEYWORD_SUPER]   = { &Parser::superExpr, nullptr, PRIMARY};
    rules[KEYWORD_NEW]   = { &Parser::newExpr, nullptr, PRIMARY};
    
    rules[PUNCT_QUESTION]  = { nullptr,          &Parser::ternary, TERNARY };
}

ExprPtr Parser::parsePrecedence(Precedence precedence) {
    Token prefixToken = advance();

    auto prefixRule = rules[prefixToken.type].prefix;

    if (!prefixRule) {
        throw Diagnostic::ParseErr("Ở đây chắc chắn chắn cần một biểu thức. Nhưng hình như bạn thiếu rồi!", prefixToken);
    }

    ExprPtr left = prefixRule(this);

    while (precedence < rules[peek().type].precedence) {
        auto infixRule = rules[peek().type].infix;

        if (!infixRule) {
            break;
        }

        auto infixToken = advance();
        left = infixRule(this, std::move(left));
    }

    return left;
}

ExprPtr Parser::expression() {
    return parsePrecedence(Precedence::NONE);
}

StmtPtr Parser::declaration() {
    StmtPtr decl = nullptr;

    if (check(KEYWORD_LET) || check(KEYWORD_CONST)) {
        bool isConstant = false;
        if (check(KEYWORD_CONST)) {
            isConstant = true;
        }
        decl = letDeclaration(advance(), isConstant);
    } else if (check(KEYWORD_FUNCTION) && next().type == IDENTIFIER) {
        decl = functionDeclaration(advance());
    } else if (check(KEYWORD_CLASS)) {
        decl = classDeclaration(advance());
    } else {
        return statement();
    }

    return decl;
}

StmtPtr Parser::statement() {
    if (check(KEYWORD_IF))       return ifStatement(advance());
    if (check(KEYWORD_WHILE))    return whileStatement(advance());
    if (check(KEYWORD_FOR))      return forStatement(advance());
    if (check(KEYWORD_RETURN))   return returnStatement(advance());
    if (check(KEYWORD_BREAK))    return breakStatement(advance());
    if (check(KEYWORD_CONTINUE)) return continueStatement(advance());
    if (check(PUNCT_LBRACE))     return blockStatement(advance());
    if (check(KEYWORD_THROW))    return throwStatement(advance());
    if (check(KEYWORD_TRY))      return tryStatement(advance());
    if (check(KEYWORD_IMPORT))   return importStatement(advance());
    if (check(KEYWORD_EXPORT))   return exportStatement(advance());
    if (check(KEYWORD_LOG))      return logStatement(advance());
    if (check(KEYWORD_DO))       return doWhileStatement(advance());
    if (check(KEYWORD_SWITCH))   return switchStatement(advance());

    return expressionStatement(peek());
}