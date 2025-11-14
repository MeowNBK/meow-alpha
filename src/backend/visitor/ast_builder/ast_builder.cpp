#include "visitor/ast_builder.hpp"
#include "common/token.hpp"
#include <stdexcept>
#include <iostream>

const Value& ASTBuilder::getObjectPropertyOrDefault(const Value& objectValue, const std::string& propName, const Value& defaultValue) {
    if (!std::holds_alternative<Object>(objectValue)) {
        throw std::runtime_error("Không phải một Object!");
    }
    const auto& objData = std::get<Object>(objectValue);
    HashKey key{Str{propName}};
    auto it = objData->pairs.find(key);
    if (it == objData->pairs.end()) {
        return defaultValue;
    }
    return it->second;
}

ASTNodePtr ASTBuilder::buildFromObject(const Value& objectValue) {
    if (!std::holds_alternative<Object>(objectValue)) {
        throw std::runtime_error("Đầu vào phải là một Object để có thể tạo AST.");
    }
    
    const Value& typeValue = getObjectProperty(objectValue, "type");
    if (!std::holds_alternative<String>(typeValue)) {
        throw std::runtime_error("Object không có thuộc tính 'type' hoặc 'type' không phải là chuỗi.");
    }

    std::string type = std::get<String>(typeValue)->str;

    if (type == "Program") {
        auto program = std::make_unique<Program>();
        const Value& bodyValue = getObjectProperty(objectValue, "body");
        if (std::holds_alternative<Array>(bodyValue)) {
            for (const auto& stmtValue : std::get<Array>(bodyValue)->elements) {
                program->body.push_back(buildStatement(stmtValue));
            }
        }
        return program;
    }

    if (type.find("Expression") != std::string::npos || type.find("Literal") != std::string::npos || type == "Identifier") {
        return buildExpression(objectValue);
    }
    if (type.find("Statement") != std::string::npos) {
        return buildStatement(objectValue);
    }
    if (type == "SwitchCase") {
        return buildSwitchCase(objectValue);
    }
    
    throw std::runtime_error("Loại node '" + type + "' không được hỗ trợ.");
}

ExprPtr ASTBuilder::buildExpression(const Value& objectValue) {
    const std::string type = std::get<String>(getObjectProperty(objectValue, "type"))->str;
    
    if (type == "IntegerLiteral") return buildIntegerLiteral(objectValue);
    if (type == "RealLiteral") return buildRealLiteral(objectValue);
    if (type == "StringLiteral") return buildStringLiteral(objectValue);
    if (type == "BooleanLiteral") return buildBooleanLiteral(objectValue);
    if (type == "NullLiteral") return buildNullLiteral(objectValue);
    if (type == "ArrayLiteral") return buildArrayLiteral(objectValue);
    if (type == "ObjectLiteral") return buildObjectLiteral(objectValue);
    if (type == "FunctionLiteral") return buildFunctionLiteral(objectValue);
    if (type == "TemplateLiteral") return buildTemplateLiteral(objectValue);
    if (type == "Identifier") return buildIdentifier(objectValue);
    if (type == "BinaryExpression") return buildBinaryExpression(objectValue);
    if (type == "UnaryExpression") return buildUnaryExpression(objectValue);
    if (type == "CallExpression") return buildCallExpression(objectValue);
    if (type == "IndexExpression") return buildIndexExpression(objectValue);
    if (type == "AssignmentExpression") return buildAssignmentExpression(objectValue);
    if (type == "TernaryExpression") return buildTernaryExpression(objectValue);
    if (type == "PropertyAccess") return buildPropertyAccess(objectValue);
    if (type == "PropertyAssignment") return buildPropertyAssignment(objectValue);
    if (type == "ThisExpression") return buildThisExpression(objectValue);
    if (type == "NewExpression") return buildNewExpression(objectValue);
    if (type == "SuperExpression") return buildSuperExpression(objectValue);
    if (type == "PrefixUpdateExpression") return buildPrefixUpdateExpression(objectValue);
    if (type == "PostfixUpdateExpression") return buildPostfixUpdateExpression(objectValue);
    if (type == "SpreadExpression") return buildSpreadExpression(objectValue);

    throw std::runtime_error("Loại expression '" + type + "' không hợp lệ.");
}

StmtPtr ASTBuilder::buildStatement(const Value& objectValue) {
    const std::string type = std::get<String>(getObjectProperty(objectValue, "type"))->str;

    if (type == "LetStatement") return buildLetStatement(objectValue);
    if (type == "ReturnStatement") return buildReturnStatement(objectValue);
    if (type == "BreakStatement") return buildBreakStatement(objectValue);
    if (type == "ContinueStatement") return buildContinueStatement(objectValue);
    if (type == "ThrowStatement") return buildThrowStatement(objectValue);
    if (type == "IfStatement") return buildIfStatement(objectValue);
    if (type == "WhileStatement") return buildWhileStatement(objectValue);
    if (type == "ForStatement") return buildForStatement(objectValue);
    if (type == "ForInStatement") return buildForInStatement(objectValue);
    if (type == "BlockStatement") return buildBlockStatement(objectValue);
    if (type == "ClassStatement") return buildClassStatement(objectValue);
    if (type == "ImportStatement") return buildImportStatement(objectValue);
    if (type == "ExportStatement") return buildExportStatement(objectValue);
    if (type == "TryStatement") return buildTryStatement(objectValue);
    if (type == "ExpressionStatement") return buildExpressionStatement(objectValue);
    if (type == "LogStatement") return buildLogStatement(objectValue);
    if (type == "SwitchStatement") return buildSwitchStatement(objectValue);
    if (type == "DoWhileStatement") return buildDoWhileStatement(objectValue);

    throw std::runtime_error("Loại statement '" + type + "' không hợp lệ.");
}

ExprPtr ASTBuilder::buildIntegerLiteral(const Value& objectValue) {
    Int value = std::get<Int>(getObjectProperty(objectValue, "value"));
    return std::make_unique<IntegerLiteral>(createDummyToken(std::to_string(value)), value);
}

ExprPtr ASTBuilder::buildRealLiteral(const Value& objectValue) {
    Real value = std::get<Real>(getObjectProperty(objectValue, "value"));
    return std::make_unique<RealLiteral>(createDummyToken(std::to_string(value)), value);
}

ExprPtr ASTBuilder::buildStringLiteral(const Value& objectValue) {

    Str value = std::get<String>(getObjectProperty(objectValue, "value"))->str;
    return std::make_unique<StringLiteral>(createDummyToken(value));
}

ExprPtr ASTBuilder::buildBooleanLiteral(const Value& objectValue) {
    Bool value = std::get<Bool>(getObjectProperty(objectValue, "value"));
    return std::make_unique<BooleanLiteral>(createDummyToken(value ? "true" : "false"), value);
}

ExprPtr ASTBuilder::buildNullLiteral(const Value& objectValue) {
    return std::make_unique<NullLiteral>(createDummyToken("null"));
}

ExprPtr ASTBuilder::buildArrayLiteral(const Value& objectValue) {
    std::vector<ExprPtr> elements;
    const Value& elementsValue = getObjectProperty(objectValue, "elements");
    if (std::holds_alternative<Array>(elementsValue)) {
        for (const auto& elem : std::get<Array>(elementsValue)->elements) {
            elements.push_back(buildExpression(elem));
        }
    }
    return std::make_unique<ArrayLiteral>(createDummyToken("["), std::move(elements));
}

ExprPtr ASTBuilder::buildObjectLiteral(const Value& objectValue) {
    std::vector<std::pair<ExprPtr, ExprPtr>> properties;
    const Value& propertiesValue = getObjectProperty(objectValue, "properties");
    if (std::holds_alternative<Array>(propertiesValue)) {
        for (const auto& prop : std::get<Array>(propertiesValue)->elements) {
            properties.emplace_back(
                buildExpression(getObjectProperty(prop, "key")),
                buildExpression(getObjectProperty(prop, "value"))
            );
        }
    }
    return std::make_unique<ObjectLiteral>(createDummyToken("{"), std::move(properties));
}

ExprPtr ASTBuilder::buildFunctionLiteral(const Value& objectValue) {
    std::vector<IdenPtr> parameters;
    const Value& paramsValue = getObjectProperty(objectValue, "parameters");
    if (std::holds_alternative<Array>(paramsValue)) {
        for (const auto& param : std::get<Array>(paramsValue)->elements) {
            parameters.push_back(std::unique_ptr<Identifier>(static_cast<Identifier*>(buildIdentifier(param).release())));
        }
    }

    IdenPtr restParam = nullptr;
    const Value& restParamValue = getObjectPropertyOrDefault(objectValue, "restParam", Value(Null{}));
    if (!std::holds_alternative<Null>(restParamValue)) {
        restParam = std::unique_ptr<Identifier>(static_cast<Identifier*>(buildIdentifier(restParamValue).release()));
    }
    
    StmtPtr body = buildStatement(getObjectProperty(objectValue, "body"));
    return std::make_unique<FunctionLiteral>(createDummyToken("fn"), std::move(parameters), std::move(body), std::move(restParam));
}

ExprPtr ASTBuilder::buildTemplateLiteral(const Value& objectValue) {
    std::vector<ExprPtr> parts;
    const Value& partsValue = getObjectProperty(objectValue, "parts");
    if (std::holds_alternative<Array>(partsValue)) {
        for (const auto& part : std::get<Array>(partsValue)->elements) {
            parts.push_back(buildExpression(part));
        }
    }
    return std::make_unique<TemplateLiteral>(createDummyToken("`"), std::move(parts));
}

ExprPtr ASTBuilder::buildIdentifier(const Value& objectValue) {

    String name = std::get<String>(getObjectProperty(objectValue, "name"));
    return std::make_unique<Identifier>(createDummyToken(name->str));
}

ExprPtr ASTBuilder::buildBinaryExpression(const Value& objectValue) {
    ExprPtr left = buildExpression(getObjectProperty(objectValue, "left"));
    String opStr = std::get<String>(getObjectProperty(objectValue, "op"));
    ExprPtr right = buildExpression(getObjectProperty(objectValue, "right"));
    
    TokenType opType = stringToTokenType(opStr->str);

    Token opToken(opType, opStr->str, "[reconstructed]", 0, 0, nullptr);
    
    return std::make_unique<BinaryExpression>(opToken, std::move(left), std::move(right));
}

ExprPtr ASTBuilder::buildUnaryExpression(const Value& objectValue) {
    String opStr = std::get<String>(getObjectProperty(objectValue, "op"));
    ExprPtr operand = buildExpression(getObjectProperty(objectValue, "operand"));

    TokenType opType = stringToTokenType(opStr->str);
    
    Token opToken(opType, opStr->str, "[reconstructed]", 0, 0, nullptr);

    return std::make_unique<UnaryExpression>(opToken, std::move(operand));
}

ExprPtr ASTBuilder::buildCallExpression(const Value& objectValue) {
    ExprPtr callee = buildExpression(getObjectProperty(objectValue, "callee"));
    std::vector<ExprPtr> args;
    const Value& argsValue = getObjectProperty(objectValue, "args");
    if (std::holds_alternative<Array>(argsValue)) {
        for (const auto& arg : std::get<Array>(argsValue)->elements) {
            args.push_back(buildExpression(arg));
        }
    }
    return std::make_unique<CallExpression>(createDummyToken("("), std::move(callee), std::move(args));
}

ExprPtr ASTBuilder::buildIndexExpression(const Value& objectValue) {
    ExprPtr object = buildExpression(getObjectProperty(objectValue, "object"));
    ExprPtr index = buildExpression(getObjectProperty(objectValue, "index"));
    return std::make_unique<IndexExpression>(createDummyToken("["), std::move(object), std::move(index));
}

ExprPtr ASTBuilder::buildAssignmentExpression(const Value& objectValue) {
    ExprPtr target = buildExpression(getObjectProperty(objectValue, "target"));
    ExprPtr value = buildExpression(getObjectProperty(objectValue, "value"));
    return std::make_unique<AssignmentExpression>(createDummyToken("="), std::move(target), std::move(value));
}

ExprPtr ASTBuilder::buildTernaryExpression(const Value& objectValue) {
    ExprPtr condition = buildExpression(getObjectProperty(objectValue, "condition"));
    ExprPtr thenBranch = buildExpression(getObjectProperty(objectValue, "thenBranch"));
    ExprPtr elseBranch = buildExpression(getObjectProperty(objectValue, "elseBranch"));
    return std::make_unique<TernaryExpression>(createDummyToken("?"), std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

ExprPtr ASTBuilder::buildPropertyAccess(const Value& objectValue) {
    ExprPtr object = buildExpression(getObjectProperty(objectValue, "object"));
    IdenPtr property = std::unique_ptr<Identifier>(static_cast<Identifier*>(buildIdentifier(getObjectProperty(objectValue, "property")).release()));
    return std::make_unique<PropertyAccess>(createDummyToken("."), std::move(object), std::move(property));
}

ExprPtr ASTBuilder::buildPropertyAssignment(const Value& objectValue) {
    ExprPtr targetObj = buildExpression(getObjectProperty(objectValue, "targetObj"));
    IdenPtr property = std::unique_ptr<Identifier>(static_cast<Identifier*>(buildIdentifier(getObjectProperty(objectValue, "property")).release()));
    ExprPtr value = buildExpression(getObjectProperty(objectValue, "value"));
    return std::make_unique<PropertyAssignment>(createDummyToken("."), std::move(targetObj), std::move(property), std::move(value));
}

ExprPtr ASTBuilder::buildThisExpression(const Value& objectValue) {
    return std::make_unique<ThisExpression>(createDummyToken("this"));
}

ExprPtr ASTBuilder::buildSuperExpression(const Value& objectValue) {
    Bool isCallable = std::get<Bool>(getObjectPropertyOrDefault(objectValue, "isCallable", Value(false)));
    IdenPtr method = nullptr;
    const Value& methodValue = getObjectPropertyOrDefault(objectValue, "method", Value(Null{}));
    if (!std::holds_alternative<Null>(methodValue)) {
        method = std::unique_ptr<Identifier>(static_cast<Identifier*>(buildIdentifier(methodValue).release()));
    }
    return std::make_unique<SuperExpression>(createDummyToken("super"), isCallable, std::move(method));
}

ExprPtr ASTBuilder::buildNewExpression(const Value& objectValue) {
    ExprPtr expression = buildExpression(getObjectProperty(objectValue, "expression"));
    return std::make_unique<NewExpression>(createDummyToken("new"), std::move(expression));
}

ExprPtr ASTBuilder::buildPrefixUpdateExpression(const Value& objectValue) {
    String opStr = std::get<String>(getObjectProperty(objectValue, "op"));
    ExprPtr operand = buildExpression(getObjectProperty(objectValue, "operand"));

    TokenType opType = stringToTokenType(opStr->str);
    
    Token opToken(opType, opStr->str, "[reconstructed]", 0, 0, nullptr);

    return std::make_unique<PrefixUpdateExpression>(opToken, std::move(operand));
}

ExprPtr ASTBuilder::buildPostfixUpdateExpression(const Value& objectValue) {
    String opStr = std::get<String>(getObjectProperty(objectValue, "op"));
    ExprPtr operand = buildExpression(getObjectProperty(objectValue, "operand"));

    TokenType opType = stringToTokenType(opStr->str);
    
    Token opToken(opType, opStr->str, "[reconstructed]", 0, 0, nullptr);
    
    return std::make_unique<PostfixUpdateExpression>(opToken, std::move(operand));
}

ExprPtr ASTBuilder::buildSpreadExpression(const Value& objectValue) {
    ExprPtr expression = buildExpression(getObjectProperty(objectValue, "expression"));
    return std::make_unique<SpreadExpression>(createDummyToken("..."), std::move(expression));
}

StmtPtr ASTBuilder::buildLetStatement(const Value& objectValue) {
    IdenPtr name = std::unique_ptr<Identifier>(static_cast<Identifier*>(buildIdentifier(getObjectProperty(objectValue, "name")).release()));
    
    ExprPtr value = nullptr;
    const Value& valueValue = getObjectPropertyOrDefault(objectValue, "value", Value(Null{}));
    if (!std::holds_alternative<Null>(valueValue)) {
        value = buildExpression(valueValue);
    }
    
    Bool isConstant = std::get<Bool>(getObjectProperty(objectValue, "isConstant"));
    return std::make_unique<LetStatement>(createDummyToken("let"), std::move(name), std::move(value), isConstant);
}

StmtPtr ASTBuilder::buildReturnStatement(const Value& objectValue) {
    ExprPtr value = nullptr;
    const Value& valueValue = getObjectPropertyOrDefault(objectValue, "value", Value(Null{}));
    if (!std::holds_alternative<Null>(valueValue)) {
        value = buildExpression(valueValue);
    }
    return std::make_unique<ReturnStatement>(createDummyToken("return"), std::move(value));
}

StmtPtr ASTBuilder::buildBreakStatement(const Value& objectValue) {
    return std::make_unique<BreakStatement>(createDummyToken("break"));
}

StmtPtr ASTBuilder::buildContinueStatement(const Value& objectValue) {
    return std::make_unique<ContinueStatement>(createDummyToken("continue"));
}

StmtPtr ASTBuilder::buildThrowStatement(const Value& objectValue) {
    ExprPtr argument = nullptr;
    const Value& argumentValue = getObjectPropertyOrDefault(objectValue, "argument", Value(Null{}));
    if (!std::holds_alternative<Null>(argumentValue)) {
        argument = buildExpression(argumentValue);
    }
    return std::make_unique<ThrowStatement>(createDummyToken("throw"), std::move(argument));
}

StmtPtr ASTBuilder::buildIfStatement(const Value& objectValue) {
    ExprPtr condition = buildExpression(getObjectProperty(objectValue, "condition"));
    StmtPtr thenBranch = buildStatement(getObjectProperty(objectValue, "thenBranch"));
    
    StmtPtr elseBranch = nullptr;
    const Value& elseBranchValue = getObjectPropertyOrDefault(objectValue, "elseBranch", Value(Null{}));
    if (!std::holds_alternative<Null>(elseBranchValue)) {
        elseBranch = buildStatement(elseBranchValue);
    }
    
    return std::make_unique<IfStatement>(createDummyToken("if"), std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

StmtPtr ASTBuilder::buildWhileStatement(const Value& objectValue) {
    ExprPtr condition = buildExpression(getObjectProperty(objectValue, "condition"));
    StmtPtr body = buildStatement(getObjectProperty(objectValue, "body"));
    return std::make_unique<WhileStatement>(createDummyToken("while"), std::move(condition), std::move(body));
}

StmtPtr ASTBuilder::buildForStatement(const Value& objectValue) {
    StmtPtr init = nullptr;
    const Value& initValue = getObjectPropertyOrDefault(objectValue, "init", Value(Null{}));
    if (!std::holds_alternative<Null>(initValue)) {
        init = buildStatement(initValue);
    }
    
    ExprPtr condition = nullptr;
    const Value& conditionValue = getObjectPropertyOrDefault(objectValue, "condition", Value(Null{}));
    if (!std::holds_alternative<Null>(conditionValue)) {
        condition = buildExpression(conditionValue);
    }
    
    ExprPtr update = nullptr;
    const Value& updateValue = getObjectPropertyOrDefault(objectValue, "update", Value(Null{}));
    if (!std::holds_alternative<Null>(updateValue)) {
        update = buildExpression(updateValue);
    }

    StmtPtr body = buildStatement(getObjectProperty(objectValue, "body"));
    return std::make_unique<ForStatement>(createDummyToken("for"), std::move(init), std::move(condition), std::move(update), std::move(body));
}

StmtPtr ASTBuilder::buildForInStatement(const Value& objectValue) {
    IdenPtr variable = std::unique_ptr<Identifier>(static_cast<Identifier*>(buildIdentifier(getObjectProperty(objectValue, "variable")).release()));
    ExprPtr collection = buildExpression(getObjectProperty(objectValue, "collection"));
    StmtPtr body = buildStatement(getObjectProperty(objectValue, "body"));
    return std::make_unique<ForInStatement>(createDummyToken("for"), std::move(variable), std::move(collection), std::move(body));
}

StmtPtr ASTBuilder::buildBlockStatement(const Value& objectValue) {
    std::vector<StmtPtr> statements;
    const Value& statementsValue = getObjectProperty(objectValue, "statements");
    if (std::holds_alternative<Array>(statementsValue)) {
        for (const auto& stmt : std::get<Array>(statementsValue)->elements) {
            statements.push_back(buildStatement(stmt));
        }
    }
    return std::make_unique<BlockStatement>(createDummyToken("{"), std::move(statements));
}

StmtPtr ASTBuilder::buildClassStatement(const Value& objectValue) {
    IdenPtr name = std::unique_ptr<Identifier>(static_cast<Identifier*>(buildIdentifier(getObjectProperty(objectValue, "name")).release()));
    
    IdenPtr superclass = nullptr;
    const Value& superclassValue = getObjectPropertyOrDefault(objectValue, "superclass", Value(Null{}));
    if (!std::holds_alternative<Null>(superclassValue)) {
        superclass = std::unique_ptr<Identifier>(static_cast<Identifier*>(buildIdentifier(superclassValue).release()));
    }
    
    std::vector<StmtPtr> methods;
    const Value& methodsValue = getObjectProperty(objectValue, "methods");
    if (std::holds_alternative<Array>(methodsValue)) {
        for (const auto& method : std::get<Array>(methodsValue)->elements) {
            methods.push_back(buildStatement(method));
        }
    }

    std::vector<StmtPtr> static_fields;
    const Value& staticFieldsValue = getObjectProperty(objectValue, "static_fields");
    if (std::holds_alternative<Array>(staticFieldsValue)) {
        for (const auto& field : std::get<Array>(staticFieldsValue)->elements) {
            static_fields.push_back(buildStatement(field));
        }
    }
    
    return std::make_unique<ClassStatement>(createDummyToken("class"), std::move(name), std::move(superclass), std::move(methods), std::move(static_fields));
}

StmtPtr ASTBuilder::buildImportStatement(const Value& objectValue) {
    ExprPtr path = buildExpression(getObjectProperty(objectValue, "path"));
    
    std::vector<IdenPtr> namedImports;
    const Value& namedImportsValue = getObjectProperty(objectValue, "namedImports");
    if (std::holds_alternative<Array>(namedImportsValue)) {
        for (const auto& imp : std::get<Array>(namedImportsValue)->elements) {
            namedImports.push_back(std::unique_ptr<Identifier>(static_cast<Identifier*>(buildIdentifier(imp).release())));
        }
    }

    IdenPtr namespaceImport = nullptr;
    const Value& namespaceImportValue = getObjectPropertyOrDefault(objectValue, "namespaceImport", Value(Null{}));
    if (!std::holds_alternative<Null>(namespaceImportValue)) {
        namespaceImport = std::unique_ptr<Identifier>(static_cast<Identifier*>(buildIdentifier(namespaceImportValue).release()));
    }

    Bool importAll = std::get<Bool>(getObjectProperty(objectValue, "importAll"));
    return std::make_unique<ImportStatement>(createDummyToken("import"), std::move(path), std::move(namedImports), std::move(namespaceImport), importAll);
}

StmtPtr ASTBuilder::buildExportStatement(const Value& objectValue) {
    StmtPtr declaration = nullptr;
    const Value& declarationValue = getObjectPropertyOrDefault(objectValue, "declaration", Value(Null{}));
    if (!std::holds_alternative<Null>(declarationValue)) {
        declaration = buildStatement(declarationValue);
    }

    std::vector<IdenPtr> specifiers;
    const Value& specifiersValue = getObjectProperty(objectValue, "specifiers");
    if (std::holds_alternative<Array>(specifiersValue)) {
        for (const auto& spec : std::get<Array>(specifiersValue)->elements) {
            specifiers.push_back(std::unique_ptr<Identifier>(static_cast<Identifier*>(buildIdentifier(spec).release())));
        }
    }
    
    return std::make_unique<ExportStatement>(createDummyToken("export"), std::move(declaration), std::move(specifiers));
}

StmtPtr ASTBuilder::buildTryStatement(const Value& objectValue) {
    StmtPtr tryBlock = buildStatement(getObjectProperty(objectValue, "tryBlock"));
    IdenPtr catchVariable = nullptr;
    const Value& catchVariableValue = getObjectPropertyOrDefault(objectValue, "catchVariable", Value(Null{}));
    if (!std::holds_alternative<Null>(catchVariableValue)) {
        catchVariable = std::unique_ptr<Identifier>(static_cast<Identifier*>(buildIdentifier(catchVariableValue).release()));
    }
    StmtPtr catchBlock = nullptr;
    const Value& catchBlockValue = getObjectPropertyOrDefault(objectValue, "catchBlock", Value(Null{}));
    if (!std::holds_alternative<Null>(catchBlockValue)) {
        catchBlock = buildStatement(catchBlockValue);
    }

    return std::make_unique<TryStatement>(createDummyToken("try"), std::move(tryBlock), std::move(catchVariable), std::move(catchBlock));
}

StmtPtr ASTBuilder::buildExpressionStatement(const Value& objectValue) {
    ExprPtr expression = buildExpression(getObjectProperty(objectValue, "expression"));
    return std::make_unique<ExpressionStatement>(createDummyToken("expr"), std::move(expression));
}

StmtPtr ASTBuilder::buildLogStatement(const Value& objectValue) {
    ExprPtr expression = buildExpression(getObjectProperty(objectValue, "expression"));
    return std::make_unique<LogStatement>(createDummyToken("log"), std::move(expression));
}

StmtPtr ASTBuilder::buildSwitchStatement(const Value& objectValue) {
    ExprPtr value = buildExpression(getObjectProperty(objectValue, "value"));
    std::vector<std::unique_ptr<SwitchCase>> cases;
    const Value& casesValue = getObjectProperty(objectValue, "cases");
    if (std::holds_alternative<Array>(casesValue)) {
        for (const auto& cas : std::get<Array>(casesValue)->elements) {
            cases.push_back(buildSwitchCase(cas));
        }
    }
    return std::make_unique<SwitchStatement>(createDummyToken("switch"), std::move(value), std::move(cases));
}

StmtPtr ASTBuilder::buildDoWhileStatement(const Value& objectValue) {
    StmtPtr body = buildStatement(getObjectProperty(objectValue, "body"));
    ExprPtr condition = buildExpression(getObjectProperty(objectValue, "condition"));
    return std::make_unique<DoWhileStatement>(createDummyToken("do"), std::move(body), std::move(condition));
}

std::unique_ptr<SwitchCase> ASTBuilder::buildSwitchCase(const Value& objectValue) {
    ExprPtr value = nullptr;
    const Value& valueValue = getObjectPropertyOrDefault(objectValue, "value", Value(Null{}));
    if (!std::holds_alternative<Null>(valueValue)) {
        value = buildExpression(valueValue);
    }
    
    std::vector<StmtPtr> statements;
    const Value& statementsValue = getObjectProperty(objectValue, "statements");
    if (std::holds_alternative<Array>(statementsValue)) {
        for (const auto& stmt : std::get<Array>(statementsValue)->elements) {
            statements.push_back(buildStatement(stmt));
        }
    }
    return std::make_unique<SwitchCase>(createDummyToken("case"), std::move(value), std::move(statements));
}



Token ASTBuilder::createDummyToken(const std::string& lexeme) {
    TokenType type = TokenType::UNKNOWN;
    return Token(type, lexeme, "[reconstructed]", 0, 0, nullptr);
}

const Value& ASTBuilder::getObjectProperty(const Value& objectValue, const std::string& propName) {
    if (!std::holds_alternative<Object>(objectValue)) {
        throw std::runtime_error("Không phải một Object!");
    }
    const auto& objData = std::get<Object>(objectValue);
    HashKey key{Str{propName}};
    auto it = objData->pairs.find(key);
    if (it == objData->pairs.end()) {
        throw std::runtime_error("Object không có thuộc tính '" + propName + "'");
    }
    return it->second;
}