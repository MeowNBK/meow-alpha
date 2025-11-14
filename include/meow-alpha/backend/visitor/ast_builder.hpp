#pragma once

#include "common/ast.hpp"
#include "runtime/value.hpp"
#include <memory>
#include <string>

class ASTBuilder {
public:
    ASTNodePtr buildFromObject(const Value& objectValue);
private:

    ExprPtr buildExpression(const Value& objectValue);
    StmtPtr buildStatement(const Value& objectValue);

    ExprPtr buildIntegerLiteral(const Value& objectValue);
    ExprPtr buildRealLiteral(const Value& objectValue);
    ExprPtr buildStringLiteral(const Value& objectValue);
    ExprPtr buildBooleanLiteral(const Value& objectValue);
    ExprPtr buildNullLiteral(const Value& objectValue);
    ExprPtr buildArrayLiteral(const Value& objectValue);
    ExprPtr buildObjectLiteral(const Value& objectValue);
    ExprPtr buildFunctionLiteral(const Value& objectValue);
    ExprPtr buildTemplateLiteral(const Value& objectValue);

    ExprPtr buildIdentifier(const Value& objectValue);
    ExprPtr buildBinaryExpression(const Value& objectValue);
    ExprPtr buildUnaryExpression(const Value& objectValue);
    ExprPtr buildCallExpression(const Value& objectValue);
    ExprPtr buildIndexExpression(const Value& objectValue);
    ExprPtr buildAssignmentExpression(const Value& objectValue);
    ExprPtr buildTernaryExpression(const Value& objectValue);
    ExprPtr buildPropertyAccess(const Value& objectValue);
    ExprPtr buildPropertyAssignment(const Value& objectValue);
    ExprPtr buildThisExpression(const Value& objectValue);
    ExprPtr buildSuperExpression(const Value& objectValue);
    ExprPtr buildNewExpression(const Value& objectValue);
    ExprPtr buildPrefixUpdateExpression(const Value& objectValue);
    ExprPtr buildPostfixUpdateExpression(const Value& objectValue);
    ExprPtr buildSpreadExpression(const Value& objectValue);

    StmtPtr buildLetStatement(const Value& objectValue);
    StmtPtr buildReturnStatement(const Value& objectValue);
    StmtPtr buildBreakStatement(const Value& objectValue);
    StmtPtr buildContinueStatement(const Value& objectValue);
    StmtPtr buildThrowStatement(const Value& objectValue);
    StmtPtr buildIfStatement(const Value& objectValue);
    StmtPtr buildWhileStatement(const Value& objectValue);
    StmtPtr buildForStatement(const Value& objectValue);
    StmtPtr buildForInStatement(const Value& objectValue);
    StmtPtr buildBlockStatement(const Value& objectValue);
    StmtPtr buildClassStatement(const Value& objectValue);
    StmtPtr buildImportStatement(const Value& objectValue);
    StmtPtr buildExportStatement(const Value& objectValue);
    StmtPtr buildTryStatement(const Value& objectValue);
    StmtPtr buildExpressionStatement(const Value& objectValue);
    StmtPtr buildLogStatement(const Value& objectValue);
    StmtPtr buildSwitchStatement(const Value& objectValue);
    StmtPtr buildDoWhileStatement(const Value& objectValue);
    std::unique_ptr<SwitchCase> buildSwitchCase(const Value& objectValue);

    Token createDummyToken(const std::string& lexeme = "");
    const Value& getObjectProperty(const Value& objectValue, const std::string& propName);
    const Value& getObjectPropertyOrDefault(const Value& objectValue, const std::string& propName, const Value& defaultValue);
};