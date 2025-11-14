#include "visitor/print_visitor.hpp"

Value PrintVisitor::createObject(std::unordered_map<HashKey, Value>&& pairs) {
    auto objData = std::make_shared<ObjectData>();
    objData->pairs = std::move(pairs);
    return Value(objData);
}

Value PrintVisitor::createArray(const std::vector<StmtPtr>& stmts) {
    auto arrayData = std::make_shared<ArrayData>();
    for (const auto& stmt : stmts) {
        arrayData->elements.push_back(stmt->accept(this));
    }
    return Value(arrayData);
}

Value PrintVisitor::createArray(const std::vector<ExprPtr>& exprs) {
    auto arrayData = std::make_shared<ArrayData>();
    for (const auto& expr : exprs) {
        arrayData->elements.push_back(expr->accept(this));
    }
    return Value(arrayData);
}

Value PrintVisitor::createArray(const std::vector<IdenPtr>& idens) {
    auto arrayData = std::make_shared<ArrayData>();
    for (const auto& iden : idens) {
        arrayData->elements.push_back(iden->accept(this));
    }
    return Value(arrayData);
}

Value PrintVisitor::createArrayForPairs(const std::vector<std::pair<ExprPtr, ExprPtr>>& pairs) {
    auto arrayData = std::make_shared<ArrayData>();
    for (const auto& pair : pairs) {
        arrayData->elements.push_back(createObject({
            {HashKey{Str{"key"}},   pair.first->accept(this)},
            {HashKey{Str{"value"}}, pair.second->accept(this)}
        }));
    }
    return Value(arrayData);
}


Value PrintVisitor::visit(Program* node) {
    return createObject({
        {HashKey{Str{"type"}}, Value(Str{"Program"})},
        {HashKey{Str{"body"}}, createArray(node->body)}
    });
}

Value PrintVisitor::visit(IntegerLiteral* node) {
    return createObject({
        {HashKey{Str{"type"}},  Value(Str{"IntegerLiteral"})},
        {HashKey{Str{"value"}}, Value(node->value)}
    });
}

Value PrintVisitor::visit(RealLiteral* node) {
    return createObject({
        {HashKey{Str{"type"}},  Value(Str{"RealLiteral"})},
        {HashKey{Str{"value"}}, Value(node->value)}
    });
}

Value PrintVisitor::visit(StringLiteral* node) {
    return createObject({
        {HashKey{Str{"type"}},  Value(Str{"StringLiteral"})},
        {HashKey{Str{"value"}}, Value(node->value)}
    });
}

Value PrintVisitor::visit(BooleanLiteral* node) {
    return createObject({
        {HashKey{Str{"type"}},  Value(Str{"BooleanLiteral"})},
        {HashKey{Str{"value"}}, Value(node->value)}
    });
}

Value PrintVisitor::visit(NullLiteral* node) {
    return createObject({
        {HashKey{Str{"type"}}, Value(Str{"NullLiteral"})},
        {HashKey{Str{"value"}}, Value(Null{})}
    });
}

Value PrintVisitor::visit(ArrayLiteral* node) {
    return createObject({
        {HashKey{Str{"type"}},     Value(Str{"ArrayLiteral"})},
        {HashKey{Str{"elements"}}, createArray(node->elements)}
    });
}

Value PrintVisitor::visit(ObjectLiteral* node) {
    return createObject({
        {HashKey{Str{"type"}},       Value(Str{"ObjectLiteral"})},
        {HashKey{Str{"properties"}}, createArrayForPairs(node->properties)}
    });
}

Value PrintVisitor::visit(FunctionLiteral* node) {
    Value body = node->body ? node->body->accept(this) : Value(Null{});
    Value restParam = node->restParam ? node->restParam->accept(this) : Value(Null{});
    return createObject({
        {HashKey{Str{"type"}},       Value(Str{"FunctionLiteral"})},
        {HashKey{Str{"parameters"}}, createArray(node->parameters)},
        {HashKey{Str{"body"}},       body},
        {HashKey{Str{"restParam"}},  restParam}
    });
}

Value PrintVisitor::visit(TemplateLiteral* node) {
    return createObject({
        {HashKey{Str{"type"}},  Value(Str{"TemplateLiteral"})},
        {HashKey{Str{"parts"}}, createArray(node->parts)}
    });
}

Value PrintVisitor::visit(Identifier* node) {
    return createObject({
        {HashKey{Str{"type"}}, Value(Str{"Identifier"})},
        {HashKey{Str{"name"}}, Value(node->name)}
    });
}

Value PrintVisitor::visit(BinaryExpression* node) {
    return createObject({
        {HashKey{Str{"type"}},  Value(Str{"BinaryExpression"})},
        {HashKey{Str{"left"}},  node->left->accept(this)},
        {HashKey{Str{"op"}},    Value(Str{tokenTypeToString(node->token.type)})},
        {HashKey{Str{"right"}}, node->right->accept(this)}
    });
}

Value PrintVisitor::visit(UnaryExpression* node) {
    return createObject({
        {HashKey{Str{"type"}},    Value(Str{"UnaryExpression"})},
        {HashKey{Str{"op"}},      Value(Str{tokenTypeToString(node->token.type)})},
        {HashKey{Str{"operand"}}, node->operand->accept(this)}
    });
}

Value PrintVisitor::visit(CallExpression* node) {
    return createObject({
        {HashKey{Str{"type"}},   Value(Str{"CallExpression"})},
        {HashKey{Str{"callee"}}, node->callee->accept(this)},
        {HashKey{Str{"args"}},   createArray(node->args)}
    });
}

Value PrintVisitor::visit(IndexExpression* node) {
    return createObject({
        {HashKey{Str{"type"}},  Value(Str{"IndexExpression"})},
        {HashKey{Str{"object"}}, node->left->accept(this)},
        {HashKey{Str{"index"}}, node->index->accept(this)}
    });
}

Value PrintVisitor::visit(AssignmentExpression* node) {
    return createObject({
        {HashKey{Str{"type"}},   Value(Str{"AssignmentExpression"})},
        {HashKey{Str{"target"}}, node->target->accept(this)},
        {HashKey{Str{"value"}},  node->value->accept(this)}
    });
}

Value PrintVisitor::visit(TernaryExpression* node) {
    Value elseBranch = node->elseBranch ? node->elseBranch->accept(this) : Value(Null{});
    return createObject({
        {HashKey{Str{"type"}},       Value(Str{"TernaryExpression"})},
        {HashKey{Str{"condition"}},  node->condition->accept(this)},
        {HashKey{Str{"thenBranch"}}, node->thenBranch->accept(this)},
        {HashKey{Str{"elseBranch"}}, elseBranch}
    });
}

Value PrintVisitor::visit(PropertyAccess* node) {
    return createObject({
        {HashKey{Str{"type"}},     Value(Str{"PropertyAccess"})},
        {HashKey{Str{"object"}},   node->object->accept(this)},
        {HashKey{Str{"property"}}, node->property->accept(this)}
    });
}

Value PrintVisitor::visit(PropertyAssignment* node) {
    return createObject({
        {HashKey{Str{"type"}},       Value(Str{"PropertyAssignment"})},
        {HashKey{Str{"targetObj"}},  node->targetObj->accept(this)},
        {HashKey{Str{"property"}},   node->property->accept(this)},
        {HashKey{Str{"value"}},      node->value->accept(this)}
    });
}

Value PrintVisitor::visit(ThisExpression* node) {
    return createObject({
        {HashKey{Str{"type"}}, Value(Str{"ThisExpression"})}
    });
}

Value PrintVisitor::visit(NewExpression* node) {
    return createObject({
        {HashKey{Str{"type"}}, Value(Str{"NewExpression"})},
        {HashKey{Str{"expression"}}, node->expression->accept(this)}
    });
}

Value PrintVisitor::visit(SuperExpression* node) {
    Value method = node->method ? node->method->accept(this) : Value(Null{});
    return createObject({
        {HashKey{Str{"type"}},       Value(Str{"SuperExpression"})},
        {HashKey{Str{"isCallable"}}, Value(node->isCallable)},
        {HashKey{Str{"method"}},     method}
    });
}

Value PrintVisitor::visit(PrefixUpdateExpression* node) {
    return createObject({
        {HashKey{Str{"type"}},    Value(Str{"PrefixUpdateExpression"})},
        {HashKey{Str{"op"}},      Value(Str{tokenTypeToString(node->token.type)})},
        {HashKey{Str{"operand"}}, node->operand->accept(this)}
    });
}

Value PrintVisitor::visit(PostfixUpdateExpression* node) {
    return createObject({
        {HashKey{Str{"type"}},    Value(Str{"PostfixUpdateExpression"})},
        {HashKey{Str{"op"}},      Value(Str{tokenTypeToString(node->token.type)})},
        {HashKey{Str{"operand"}}, node->operand->accept(this)}
    });
}

Value PrintVisitor::visit(SpreadExpression* node) {
    return createObject({
        {HashKey{Str{"type"}},       Value(Str{"SpreadExpression"})},
        {HashKey{Str{"expression"}}, node->expression->accept(this)}
    });
}

Value PrintVisitor::visit(LetStatement* node) {
    Value value = node->value ? node->value->accept(this) : Value(Null{});
    return createObject({
        {HashKey{Str{"type"}},       Value(Str{"LetStatement"})},
        {HashKey{Str{"name"}},       node->name->accept(this)},
        {HashKey{Str{"value"}},      value},
        {HashKey{Str{"isConstant"}}, Value(node->isConstant)}
    });
}

Value PrintVisitor::visit(ReturnStatement* node) {
    Value value = node->value ? node->value->accept(this) : Value(Null{});
    return createObject({
        {HashKey{Str{"type"}},  Value(Str{"ReturnStatement"})},
        {HashKey{Str{"value"}}, value}
    });
}

Value PrintVisitor::visit(BreakStatement* node) {
    return createObject({
        {HashKey{Str{"type"}}, Value(Str{"BreakStatement"})}
    });
}

Value PrintVisitor::visit(ContinueStatement* node) {
    return createObject({
        {HashKey{Str{"type"}}, Value(Str{"ContinueStatement"})}
    });
}

Value PrintVisitor::visit(ThrowStatement* node) {
    Value arg = node->argument ? node->argument->accept(this) : Value(Null{});
    return createObject({
        {HashKey{Str{"type"}},     Value(Str{"ThrowStatement"})},
        {HashKey{Str{"argument"}}, arg}
    });
}

Value PrintVisitor::visit(IfStatement* node) {
    Value elseBranch = node->elseBranch ? node->elseBranch->accept(this) : Value(Null{});
    return createObject({
        {HashKey{Str{"type"}},       Value(Str{"IfStatement"})},
        {HashKey{Str{"condition"}},  node->condition->accept(this)},
        {HashKey{Str{"thenBranch"}}, node->thenBranch->accept(this)},
        {HashKey{Str{"elseBranch"}}, elseBranch}
    });
}

Value PrintVisitor::visit(WhileStatement* node) {
    return createObject({
        {HashKey{Str{"type"}},      Value(Str{"WhileStatement"})},
        {HashKey{Str{"condition"}}, node->condition->accept(this)},
        {HashKey{Str{"body"}},      node->body->accept(this)}
    });
}

Value PrintVisitor::visit(ForStatement* node) {
    Value init = node->init ? node->init->accept(this) : Value(Null{});
    Value condition = node->condition ? node->condition->accept(this) : Value(Null{});
    Value update = node->update ? node->update->accept(this) : Value(Null{});
    return createObject({
        {HashKey{Str{"type"}},      Value(Str{"ForStatement"})},
        {HashKey{Str{"init"}},      init},
        {HashKey{Str{"condition"}}, condition},
        {HashKey{Str{"update"}},    update},
        {HashKey{Str{"body"}},      node->body->accept(this)}
    });
}

Value PrintVisitor::visit(ForInStatement* node) {
    return createObject({
        {HashKey{Str{"type"}},      Value(Str{"ForInStatement"})},
        {HashKey{Str{"variable"}},  node->variable->accept(this)},
        {HashKey{Str{"collection"}},node->collection->accept(this)},
        {HashKey{Str{"body"}},      node->body->accept(this)}
    });
}

Value PrintVisitor::visit(BlockStatement* node) {
    return createObject({
        {HashKey{Str{"type"}},       Value(Str{"BlockStatement"})},
        {HashKey{Str{"statements"}}, createArray(node->statements)}
    });
}

Value PrintVisitor::visit(ClassStatement* node) {
    Value superclass = node->superclass ? node->superclass->accept(this) : Value(Null{});
    return createObject({
        {HashKey{Str{"type"}},         Value(Str{"ClassStatement"})},
        {HashKey{Str{"name"}},         node->name->accept(this)},
        {HashKey{Str{"superclass"}},   superclass},
        {HashKey{Str{"methods"}},      createArray(node->methods)},
        {HashKey{Str{"static_fields"}}, createArray(node->static_fields)}
    });
}

Value PrintVisitor::visit(ImportStatement* node) {
    Value path = node->path ? node->path->accept(this) : Value(Null{});
    Value namespaceImport = node->namespaceImport ? node->namespaceImport->accept(this) : Value(Null{});
    return createObject({
        {HashKey{Str{"type"}},           Value(Str{"ImportStatement"})},
        {HashKey{Str{"path"}},           path},
        {HashKey{Str{"namedImports"}},   createArray(node->namedImports)},
        {HashKey{Str{"namespaceImport"}}, namespaceImport},
        {HashKey{Str{"importAll"}},      Value(node->importAll)}
    });
}

Value PrintVisitor::visit(ExportStatement* node) {
    Value declaration = node->declaration ? node->declaration->accept(this) : Value(Null{});
    return createObject({
        {HashKey{Str{"type"}},        Value(Str{"ExportStatement"})},
        {HashKey{Str{"declaration"}}, declaration},
        {HashKey{Str{"specifiers"}},  createArray(node->specifiers)}
    });
}

Value PrintVisitor::visit(TryStatement* node) {
    Value catchVar = node->catchVariable ? node->catchVariable->accept(this) : Value(Null{});
    Value catchBlock = node->catchBlock ? node->catchBlock->accept(this) : Value(Null{});
    return createObject({
        {HashKey{Str{"type"}},          Value(Str{"TryStatement"})},
        {HashKey{Str{"tryBlock"}},      node->tryBlock->accept(this)},
        {HashKey{Str{"catchVariable"}}, catchVar},
        {HashKey{Str{"catchBlock"}},    catchBlock}
    });
}

Value PrintVisitor::visit(ExpressionStatement* node) {
    return createObject({
        {HashKey{Str{"type"}},       Value(Str{"ExpressionStatement"})},
        {HashKey{Str{"expression"}}, node->expression->accept(this)}
    });
}

Value PrintVisitor::visit(LogStatement* node) {
    return createObject({
        {HashKey{Str{"type"}},       Value(Str{"LogStatement"})},
        {HashKey{Str{"expression"}}, node->expression->accept(this)}
    });
}

Value PrintVisitor::visit(SwitchCase* node) {
    Value value = node->value ? node->value->accept(this) : Value(Null{});
    return createObject({
        {HashKey{Str{"type"}},       Value(Str{"SwitchCase"})},
        {HashKey{Str{"value"}},      value},
        {HashKey{Str{"statements"}}, createArray(node->statements)}
    });
}

Value PrintVisitor::visit(SwitchStatement* node) {
    auto cases = std::make_shared<ArrayData>();
    for (const auto& cas : node->cases) {
        cases->elements.push_back(cas->accept(this));
    }
    return createObject({
        {HashKey{Str{"type"}},    Value(Str{"SwitchStatement"})},
        {HashKey{Str{"value"}},   node->value->accept(this)},
        {HashKey{Str{"cases"}},   Value(cases)}
    });
}

Value PrintVisitor::visit(DoWhileStatement* node) {
    return createObject({
        {HashKey{Str{"type"}},      Value(Str{"DoWhileStatement"})},
        {HashKey{Str{"body"}},      node->body->accept(this)},
        {HashKey{Str{"condition"}}, node->condition->accept(this)}
    });
}