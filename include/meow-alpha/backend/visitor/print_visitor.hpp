#pragma once

#include "visitor/visitor.hpp"
#include "runtime/value.hpp"
#include "common/ast.hpp"
#include <string>

class PrintVisitor : public Visitor {
public:
    virtual ~PrintVisitor() = default;

    Value visit(Program* node) override;

    Value visit(IntegerLiteral* node) override;
    Value visit(RealLiteral* node) override;
    Value visit(StringLiteral* node) override;
    Value visit(BooleanLiteral* node) override;
    Value visit(NullLiteral* node) override;
    Value visit(ArrayLiteral* node) override;
    Value visit(ObjectLiteral* node) override;
    Value visit(FunctionLiteral* node) override;
    Value visit(TemplateLiteral* node) override;

    Value visit(Identifier* node) override;
    Value visit(BinaryExpression* node) override;
    Value visit(UnaryExpression* node) override;
    Value visit(CallExpression* node) override;
    Value visit(IndexExpression* node) override;
    Value visit(AssignmentExpression* node) override;
    Value visit(TernaryExpression* node) override;
    Value visit(PropertyAccess* node) override;
    Value visit(PropertyAssignment* node) override;
    Value visit(ThisExpression* node) override;
    Value visit(SuperExpression* node) override;
    Value visit(NewExpression* node) override;
    Value visit(PrefixUpdateExpression* node) override;
    Value visit(PostfixUpdateExpression* node) override;
    Value visit(SpreadExpression* node) override;

    Value visit(LetStatement* node) override;
    Value visit(ReturnStatement* node) override;
    Value visit(BreakStatement* node) override;
    Value visit(ContinueStatement* node) override;
    Value visit(ThrowStatement* node) override;
    Value visit(IfStatement* node) override;
    Value visit(WhileStatement* node) override;
    Value visit(ForStatement* node) override;
    Value visit(ForInStatement* node) override;
    Value visit(BlockStatement* node) override;
    Value visit(ClassStatement* node) override;
    Value visit(ImportStatement* node) override;
    Value visit(ExportStatement* node) override;
    Value visit(TryStatement* node) override;
    Value visit(ExpressionStatement* node) override;
    Value visit(LogStatement* node) override;
    Value visit(SwitchCase* node) override;
    Value visit(SwitchStatement* node) override;
    Value visit(DoWhileStatement* node) override;

private:
    Value createObject(std::unordered_map<HashKey, Value>&& pairs);
    Value createArray(const std::vector<StmtPtr>& stmts);
    Value createArray(const std::vector<ExprPtr>& exprs);
    Value createArray(const std::vector<IdenPtr>& idens);
    Value createArrayForPairs(const std::vector<std::pair<ExprPtr, ExprPtr>>& pairs);
};