#pragma once

#include "runtime/value.hpp"

struct ASTNode;
struct Expression;
struct Statement;
struct Program;

struct IntegerLiteral;
struct RealLiteral;
struct StringLiteral;
struct BooleanLiteral;
struct NullLiteral;
struct ArrayLiteral;
struct ObjectLiteral;
struct FunctionLiteral;
struct TemplateLiteral;

struct Identifier;
struct BinaryExpression;
struct UnaryExpression;
struct CallExpression;
struct IndexExpression;
struct AssignmentExpression;
struct TernaryExpression;
struct PropertyAccess;
struct PropertyAssignment;
struct ThisExpression;
struct SuperExpression;
struct NewExpression;
struct PrefixUpdateExpression;
struct PostfixUpdateExpression;
struct SpreadExpression;

struct LetStatement;
struct ReturnStatement;
struct BreakStatement;
struct ContinueStatement;
struct ThrowStatement;
struct IfStatement;
struct WhileStatement;
struct ForStatement;
struct ForInStatement;
struct BlockStatement;
struct ClassStatement;
struct ImportStatement;
struct ExportStatement;
struct TryStatement;
struct ExpressionStatement;
struct LogStatement;
struct SwitchCase;
struct SwitchStatement;
struct DoWhileStatement;

class Visitor {
public:
    virtual ~Visitor() = default;

    virtual Value visit(Program* node) = 0;

    virtual Value visit(IntegerLiteral* node) = 0;
    virtual Value visit(RealLiteral* node) = 0;
    virtual Value visit(StringLiteral* node) = 0;
    virtual Value visit(BooleanLiteral* node) = 0;
    virtual Value visit(NullLiteral* node) = 0;
    virtual Value visit(ArrayLiteral* node) = 0;
    virtual Value visit(ObjectLiteral* node) = 0;
    virtual Value visit(FunctionLiteral* node) = 0;
    virtual Value visit(TemplateLiteral* node) = 0;

    virtual Value visit(Identifier* node) = 0;
    virtual Value visit(BinaryExpression* node) = 0;
    virtual Value visit(UnaryExpression* node) = 0;
    virtual Value visit(CallExpression* node) = 0;
    virtual Value visit(IndexExpression* node) = 0;
    virtual Value visit(AssignmentExpression* node) = 0;
    virtual Value visit(TernaryExpression* node) = 0;
    virtual Value visit(PropertyAccess* node) = 0;
    virtual Value visit(PropertyAssignment* node) = 0;
    virtual Value visit(ThisExpression* node) = 0;
    virtual Value visit(SuperExpression* node) = 0;
    virtual Value visit(NewExpression* node) = 0;
    virtual Value visit(PrefixUpdateExpression* node) = 0;
    virtual Value visit(PostfixUpdateExpression* node) = 0;
    virtual Value visit(SpreadExpression* node) = 0;

    virtual Value visit(LetStatement* node) = 0;
    virtual Value visit(ReturnStatement* node) = 0;
    virtual Value visit(BreakStatement* node) = 0;
    virtual Value visit(ContinueStatement* node) = 0;
    virtual Value visit(ThrowStatement* node) = 0;
    virtual Value visit(IfStatement* node) = 0;
    virtual Value visit(WhileStatement* node) = 0;
    virtual Value visit(ForStatement* node) = 0;
    virtual Value visit(ForInStatement* node) = 0;
    virtual Value visit(BlockStatement* node) = 0;
    virtual Value visit(ClassStatement* node) = 0;
    virtual Value visit(ImportStatement* node) = 0;
    virtual Value visit(ExportStatement* node) = 0;
    virtual Value visit(TryStatement* node) = 0;
    virtual Value visit(ExpressionStatement* node) = 0;
    virtual Value visit(LogStatement* node) = 0;
    virtual Value visit(SwitchCase* node) = 0;
    virtual Value visit(SwitchStatement* node) = 0;
    virtual Value visit(DoWhileStatement* node) = 0;
};