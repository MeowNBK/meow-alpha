#pragma once

#include "common/token.hpp"
#include "visitor/visitor.hpp"
#include <memory>
#include <cstdint>
#include <vector>
#include <string>
#include <utility>

struct ASTNode;
struct Expression;
struct Statement;

using ASTNodePtr = std::unique_ptr<ASTNode>;
using ExprPtr = std::unique_ptr<Expression>;
using StmtPtr = std::unique_ptr<Statement>;

struct Identifier;

using IdenPtr = std::unique_ptr<Identifier>;

enum class NodeType {
    PROGRAM,

    EXPR_IDENTIFIER,
    EXPR_BINARY,
    EXPR_UNARY,
    EXPR_CALL,
    EXPR_INDEX,
    EXPR_ASSIGN,
    EXPR_TERNARY,
    EXPR_PROPERTY_ACCESS,
    EXPR_PROPERTY_ASSIGN, 
    EXPR_THIS,
    EXPR_SUPER,
    EXPR_NEW,
    EXPR_UPDATE_PREFIX,
    EXPR_UPDATE_POSTFIX,
    EXPR_SPREAD,

    STMT_LET,
    STMT_RETURN,
    STMT_BREAK,
    STMT_CONTINUE,
    STMT_IF,
    STMT_WHILE,
    STMT_FOR,
    STMT_FOR_IN,
    STMT_BLOCK,
    STMT_EXPR,
    STMT_CLASS,
    STMT_IMPORT,
    STMT_EXPORT,
    STMT_TRY,
    STMT_THROW,
    STMT_SWITCH,
    STMT_DO_WHILE,
    STMT_LOG,

    EXPR_LITERAL_INTEGER,
    EXPR_LITERAL_REAL,
    EXPR_LITERAL_STRING,
    EXPR_LITERAL_BOOLEAN,
    EXPR_LITERAL_NULL,
    EXPR_LITERAL_ARRAY,
    EXPR_LITERAL_OBJECT,
    EXPR_LITERAL_FUNCTION,
    EXPR_LITERAL_TEMPLATE,

    NODE_SWITCH_CASE,
};

using enum NodeType;

struct ASTNode {
    NodeType type;
    Token token;

    ASTNode(NodeType type, Token token): type(type), token(std::move(token)) {}

    virtual ~ASTNode() = default;

    virtual Value accept(Visitor* visitor) = 0;
};

struct Expression : ASTNode {
    Expression(NodeType type, Token token): ASTNode(type, std::move(token)) {}
};

struct Statement : ASTNode {
    Statement(NodeType type, Token token): ASTNode(type, std::move(token)) {}
};


struct Identifier: Expression {
    std::string name;
    Identifier(Token token): Expression(EXPR_IDENTIFIER, std::move(token)), name(this->token.lexeme) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct BinaryExpression: Expression {
    ExprPtr left;
    TokenType op;
    ExprPtr right;

    BinaryExpression(Token token, ExprPtr l, ExprPtr r): Expression(EXPR_BINARY, std::move(token)), left(std::move(l)), op(this->token.type), right(std::move(r)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct UnaryExpression: Expression {
    TokenType op;
    ExprPtr operand;

    UnaryExpression(Token token, ExprPtr ope): Expression(EXPR_UNARY, std::move(token)), op(this->token.type), operand(std::move(ope)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct CallExpression: Expression {
    ExprPtr callee;
    std::vector<ExprPtr> args;

    CallExpression(Token token, ExprPtr callee, std::vector<ExprPtr> args): Expression(EXPR_CALL, std::move(token)), callee(std::move(callee)), args(std::move(args)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct IndexExpression: Expression {
    ExprPtr left;
    ExprPtr index;

    IndexExpression(Token token, ExprPtr l, ExprPtr i): Expression(EXPR_INDEX, std::move(token)), left(std::move(l)), index(std::move(i)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct AssignmentExpression: Expression {
    ExprPtr target;
    ExprPtr value;

    AssignmentExpression(Token token, ExprPtr t, ExprPtr v): Expression(EXPR_ASSIGN, std::move(token)), target(std::move(t)), value(std::move(v)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct TernaryExpression: Expression {
    ExprPtr condition, thenBranch, elseBranch;

    TernaryExpression(Token token, ExprPtr cond, ExprPtr tb, ExprPtr eb): Expression(EXPR_TERNARY, std::move(token)), condition(std::move(cond)), thenBranch(std::move(tb)), elseBranch(std::move(eb)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct PropertyAccess: Expression {
    ExprPtr object;
    IdenPtr property;

    PropertyAccess(Token token, ExprPtr obj, IdenPtr prop)
        : Expression(EXPR_PROPERTY_ACCESS, std::move(token)), object(std::move(obj)), property(std::move(prop)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct PropertyAssignment: Expression {
    ExprPtr targetObj;
    IdenPtr property;
    ExprPtr value;

    PropertyAssignment(Token token, ExprPtr target, IdenPtr prop, ExprPtr val)
        : Expression(EXPR_PROPERTY_ASSIGN, std::move(token)), targetObj(std::move(target)), property(std::move(prop)), value(std::move(val)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct ThisExpression: Expression {
    ThisExpression(Token token): Expression(EXPR_THIS, std::move(token)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct SuperExpression: Expression {
    bool isCallable;
    IdenPtr method;
    SuperExpression(Token token, bool callable = false, IdenPtr m = nullptr): Expression(EXPR_SUPER, std::move(token)), isCallable(callable), method(std::move(m)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct NewExpression: Expression {
    ExprPtr expression;
    NewExpression(Token token, ExprPtr expr): Expression(EXPR_NEW, std::move(token)), expression(std::move(expr)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct PrefixUpdateExpression : Expression {
    TokenType op;
    ExprPtr operand;

    PrefixUpdateExpression(Token token, ExprPtr operand)
        : Expression(EXPR_UPDATE_PREFIX, std::move(token)), op(this->token.type), operand(std::move(operand)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct PostfixUpdateExpression : Expression {
    TokenType op;
    ExprPtr operand;

    PostfixUpdateExpression(Token token, ExprPtr operand)
        : Expression(EXPR_UPDATE_POSTFIX, std::move(token)), op(this->token.type), operand(std::move(operand)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct SpreadExpression : Expression {
    ExprPtr expression;

    SpreadExpression(Token token, ExprPtr expr)
        : Expression(EXPR_SPREAD, std::move(token)), expression(std::move(expr)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct IntegerLiteral: Expression {
    int64_t value;
    IntegerLiteral(Token token, int64_t value): Expression(EXPR_LITERAL_INTEGER, std::move(token)), value(value) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct RealLiteral: Expression {
    double value;
    RealLiteral(Token token, double value): Expression(EXPR_LITERAL_REAL, std::move(token)), value(value) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct StringLiteral: Expression {
    std::string value;
    StringLiteral(Token token): Expression(EXPR_LITERAL_STRING, std::move(token)), value(this->token.lexeme) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct BooleanLiteral: Expression {
    bool value;
    BooleanLiteral(Token token, bool value): Expression(EXPR_LITERAL_BOOLEAN, std::move(token)), value(value) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct NullLiteral: Expression {
    NullLiteral(Token token): Expression(EXPR_LITERAL_NULL, std::move(token)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct ArrayLiteral: Expression {
    std::vector<ExprPtr> elements;

    ArrayLiteral(Token token, std::vector<ExprPtr> elems): Expression(EXPR_LITERAL_ARRAY, std::move(token)), elements(std::move(elems)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct ObjectLiteral: Expression {
    std::vector<std::pair<ExprPtr, ExprPtr>> properties;

    ObjectLiteral(Token token, std::vector<std::pair<ExprPtr, ExprPtr>> props)
        : Expression(EXPR_LITERAL_OBJECT, std::move(token)), properties(std::move(props)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct FunctionLiteral : Expression {
    std::vector<IdenPtr> parameters;
    StmtPtr body;
    IdenPtr restParam;

    FunctionLiteral(Token token, std::vector<IdenPtr> p, StmtPtr b, IdenPtr r = nullptr): Expression(EXPR_LITERAL_FUNCTION, std::move(token)), parameters(std::move(p)), body(std::move(b)), restParam(std::move(r)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct TemplateLiteral: Expression {
    std::vector<ExprPtr> parts;

    TemplateLiteral(Token token, std::vector<ExprPtr> p): Expression(EXPR_LITERAL_TEMPLATE, std::move(token)), parts(std::move(p)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct LetStatement: Statement {
    IdenPtr name;
    ExprPtr value;
    bool isConstant;

    LetStatement(Token token, IdenPtr n, ExprPtr v, bool isConst = false): Statement(STMT_LET, std::move(token)), name(std::move(n)), value(std::move(v)), isConstant(isConst) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct ReturnStatement: Statement {
    ExprPtr value;

    ReturnStatement(Token token, ExprPtr v = nullptr): Statement(STMT_RETURN, std::move(token)), value(std::move(v)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct BreakStatement: Statement {
    BreakStatement(Token token): Statement(STMT_BREAK, std::move(token)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct ContinueStatement: Statement {
    ContinueStatement(Token token): Statement(STMT_CONTINUE, std::move(token)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct ThrowStatement: Statement {
    ExprPtr argument;

    ThrowStatement(Token token, ExprPtr arg = nullptr): Statement(STMT_THROW, std::move(token)), argument(std::move(arg)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct IfStatement: Statement {
    ExprPtr condition;
    StmtPtr thenBranch;
    StmtPtr elseBranch;

    IfStatement(Token token, ExprPtr c, StmtPtr t, StmtPtr e = nullptr): Statement(STMT_IF, std::move(token)), condition(std::move(c)), thenBranch(std::move(t)), elseBranch(std::move(e)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct WhileStatement: Statement {
    ExprPtr condition;
    StmtPtr body;

    WhileStatement(Token token, ExprPtr c, StmtPtr b): Statement(STMT_WHILE, std::move(token)), condition(std::move(c)), body(std::move(b)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct ForStatement: Statement {
    StmtPtr init;
    ExprPtr condition;
    ExprPtr update;
    StmtPtr body;

    ForStatement(Token token, StmtPtr i, ExprPtr c, ExprPtr u, StmtPtr b): Statement(STMT_FOR, std::move(token)), init(std::move(i)), condition(std::move(c)), update(std::move(u)), body(std::move(b)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct ForInStatement: Statement {
    IdenPtr variable;
    ExprPtr collection;
    StmtPtr body;

    ForInStatement(Token token, IdenPtr var, ExprPtr collect, StmtPtr stmt): Statement(STMT_FOR_IN, std::move(token)), variable(std::move(var)), collection(std::move(collect)), body(std::move(stmt)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct BlockStatement : Statement {
    std::vector<StmtPtr> statements;

    BlockStatement(Token token) : Statement(STMT_BLOCK, std::move(token)) {}
    BlockStatement(Token token, std::vector<StmtPtr> s): Statement(STMT_BLOCK, std::move(token)), statements(std::move(s)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct ClassStatement: Statement {
    IdenPtr name;
    IdenPtr superclass;
    std::vector<StmtPtr> methods;
    std::vector<StmtPtr> static_fields;

    ClassStatement(Token token, IdenPtr n, IdenPtr s, std::vector<StmtPtr> m, std::vector<StmtPtr> fields)
        : Statement(STMT_CLASS, std::move(token)), name(std::move(n)), superclass(std::move(s)), methods(std::move(m)), static_fields(std::move(fields)) {}
    
    ClassStatement(Token token, IdenPtr n, std::vector<StmtPtr> m, std::vector<StmtPtr> fields)
        : Statement(STMT_CLASS, std::move(token)), name(std::move(n)), superclass(nullptr), methods(std::move(m)), static_fields(std::move(fields)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct ImportStatement: Statement {
    ExprPtr path;
    std::vector<IdenPtr> namedImports;
    IdenPtr namespaceImport;

    bool importAll;

    ImportStatement(Token token, ExprPtr p, std::vector<IdenPtr> imports, IdenPtr nspace, bool all = false)
        : Statement(STMT_IMPORT, std::move(token)), path(std::move(p)), namedImports(std::move(imports)), namespaceImport(std::move(nspace)), importAll(all) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct ExportStatement: Statement {
    StmtPtr declaration;

    std::vector<IdenPtr> specifiers;

    ExportStatement(Token token, StmtPtr decl, std::vector<IdenPtr> spec)
        : Statement(STMT_EXPORT, std::move(token)), declaration(std::move(decl)), specifiers(std::move(spec)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct TryStatement: Statement {
    StmtPtr tryBlock;
    IdenPtr catchVariable;
    StmtPtr catchBlock;

    TryStatement(Token token, StmtPtr tb, IdenPtr cv, StmtPtr cb)
        : Statement(STMT_TRY, std::move(token)), tryBlock(std::move(tb)), catchVariable(std::move(cv)), catchBlock(std::move(cb)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct SwitchCase: ASTNode {
    ExprPtr value;
    std::vector<StmtPtr> statements;

    SwitchCase(Token token, ExprPtr val, std::vector<StmtPtr> stmts): ASTNode(NODE_SWITCH_CASE, std::move(token)), value(std::move(val)), statements(std::move(stmts)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct SwitchStatement: Statement {
    ExprPtr value;
    std::vector<std::unique_ptr<SwitchCase>> cases;

    SwitchStatement(Token token, ExprPtr val, std::vector<std::unique_ptr<SwitchCase>> c): Statement(STMT_SWITCH, std::move(token)), value(std::move(val)), cases(std::move(c)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct ExpressionStatement: Statement {
    ExprPtr expression;
    ExpressionStatement(Token token, ExprPtr v): Statement(STMT_EXPR, std::move(token)), expression(std::move(v)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct DoWhileStatement : Statement {
    StmtPtr body;
    ExprPtr condition;

    DoWhileStatement(Token token, StmtPtr b, ExprPtr c): Statement(STMT_DO_WHILE, std::move(token)), body(std::move(b)), condition(std::move(c)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};

struct LogStatement: Statement {
    ExprPtr expression;
    LogStatement(Token token, ExprPtr v): Statement(STMT_LOG, std::move(token)), expression(std::move(v)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};


struct Program: ASTNode {
    std::vector<StmtPtr> body;
    Program(): ASTNode(PROGRAM, Token(TokenType::UNKNOWN, "[root]", "[unknown file]", 0, 0, nullptr)) {}
    Program(std::vector<StmtPtr> stmts) : ASTNode(PROGRAM, Token(TokenType::UNKNOWN, "[root]", "[unknown file]", 0, 0, nullptr)), body(std::move(stmts)) {}

    Value accept(Visitor* visitor) override {
        return visitor->visit(this);
    }
};