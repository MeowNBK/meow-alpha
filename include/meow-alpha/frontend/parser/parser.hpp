#pragma once

#include "common/ast.hpp"
#include "diagnostics/diagnostic.hpp"
#include "parser/parse_rule.hpp"
#include <format>
#include <vector>
#include <memory>
#include <initializer_list>
#include <stdexcept>
#include <unordered_map>
#include <functional>
#include <sstream>

namespace std {
    template<>
    struct hash<TokenType> {
        size_t operator()(const TokenType &type) const {
            return static_cast<size_t>(type);
        }
    };
}

class Parser {
public:
    Parser(const std::vector<Token> &tokens);
    std::unique_ptr<Program> parseProgram();

private:
    const std::vector<Token> &tokens;
    size_t current;
    std::unordered_map<TokenType, ParseRule> rules;

    static ExprPtr literal(Parser *parser);
    static ExprPtr arrayLiteral(Parser* parser);
    static ExprPtr functionLiteral(Parser* parser);
    static ExprPtr objectLiteral(Parser* parser);
    static ExprPtr templateLiteral(Parser* parser);
    static ExprPtr unary(Parser *parser);
    static ExprPtr identifier(Parser *parser);
    static ExprPtr grouping(Parser *parser);
    static ExprPtr thisExpr(Parser* parser);
    static ExprPtr superExpr(Parser* parser);
    static ExprPtr newExpr(Parser* parser);
    static ExprPtr spreadExpr(Parser* parser);
    static ExprPtr prefixUpdate(Parser* parser);

    static ExprPtr binary(Parser *parser, ExprPtr left);
    static ExprPtr assignment(Parser *parser, ExprPtr left);
    static ExprPtr call(Parser *parser, ExprPtr left);
    static ExprPtr index(Parser *parser, ExprPtr left);
    static ExprPtr access(Parser *parser, ExprPtr left);
    static ExprPtr ternary(Parser *parser, ExprPtr left);
    static ExprPtr postfixUpdate(Parser* parser, ExprPtr left);

    ExprPtr parsePrecedence(Precedence precedence);

    ExprPtr parseFunctionTail(Token token);

    void initRules();

    bool isAtEnd() const;
    const Token& peek() const;
    const Token& previous() const;
    const Token& next() const;
    const Token& advance();
    bool check(TokenType type) const;
    bool match(std::initializer_list<TokenType> types);
    const Token& consume(TokenType type, const std::string &errMsg);
    void synchronize();

    StmtPtr statement();
    StmtPtr declaration();
    ExprPtr expression();

    StmtPtr letDeclaration(Token token, bool isConstant);
    StmtPtr functionDeclaration(Token token);
    StmtPtr classDeclaration(Token token);

    StmtPtr ifStatement(Token token);
    StmtPtr whileStatement(Token token);
    StmtPtr forStatement(Token token);
    StmtPtr returnStatement(Token token);
    StmtPtr breakStatement(Token token);
    StmtPtr continueStatement(Token token);
    StmtPtr blockStatement(Token token);
    StmtPtr throwStatement(Token token);
    StmtPtr tryStatement(Token token);
    StmtPtr importStatement(Token token);
    StmtPtr exportStatement(Token token);
    StmtPtr logStatement(Token token);
    StmtPtr switchStatement(Token token);
    StmtPtr doWhileStatement(Token token);

    StmtPtr expressionStatement(Token token);
};
