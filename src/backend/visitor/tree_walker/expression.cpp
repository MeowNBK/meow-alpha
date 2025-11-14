#include "visitor/tree_walker.hpp"
#include "common/ast.hpp"
#include "diagnostics/diagnostic.hpp"

#include <functional>
#include <sstream>
#include <type_traits>

Value TreeWalker::visit(Identifier* node) {
    return env->find(node->name);
}

Value TreeWalker::visit(UnaryExpression* node) {
    Value right = evaluate(node->operand.get());

    if (auto opFunc = opDispatcher->find(node->op, right)) {
        return (*opFunc)(right);
    }

    std::ostringstream os;

    os << "Toán tử một ngôi '" << node->token.lexeme << "' không hợp lệ cho phép toán này: '" << right << "'\n";
    
    auto diag = Diagnostic::RuntimeErr(os.str(), node->token);
    throwRuntimeErr(node->token, diag.str());

    return Value(Null{});
}

Value TreeWalker::visit(BinaryExpression* node) {
    using enum TokenType;
    if (node->op == OP_LOGICAL_OR) {
        Value left = evaluate(node->left.get());
        if (isTruthy(left)) return left;
        return evaluate(node->right.get());
    }
    if (node->op == OP_LOGICAL_AND) {
        Value left = evaluate(node->left.get());
        if (!isTruthy(left)) return left;
        return evaluate(node->right.get());
    }

    if (node->op == OP_NULLISH) {
        Value left = evaluate(node->left.get());
        if (!std::holds_alternative<Null>(left)) {
            return left;
        }
        return evaluate(node->right.get());
    }

    Value left = evaluate(node->left.get());
    Value right = evaluate(node->right.get());

    if (auto opFunc = opDispatcher->find(node->op, left, right)) {
        return (*opFunc)(left, right);
    }

    std::ostringstream os;

    os << "Toán tử hai ngôi '" << node->token.lexeme << "' không hợp lệ cho phép toán với vế trái: '" << left << "' và right: '" << right << "'\n";

    auto diag = Diagnostic::RuntimeErr(os.str(), node->token);
    throwRuntimeErr(node->token, diag.str());

    return Value(Null{});
}

Value TreeWalker::visit(CallExpression* node) {
    Value callee = evaluate(node->callee.get());
    
    std::vector<Value> args;
    for (const auto& arg : node->args) {
        if (auto spread = dynamic_cast<SpreadExpression*>(arg.get())) {
            Value collection = evaluate(spread->expression.get());
            
            if (auto iterable = toIterable(collection)) {
                auto iterator = iterable->makeIterator();
                while (iterator->hasNext()) {
                    args.push_back(iterator->next());
                }
            } else {
                throwRuntimeErr(spread->token, "Toán tử '...' chỉ có thể dùng với các kiểu có thể duyệt qua (iterable).");
            }
        } else {
            args.push_back(evaluate(arg.get()));
        }
    }
    
    try {
        return this->call(callee, args); 
    } catch (FunctionException &e) {
        std::ostringstream os;
        os << "[DEBUG] Lỗi ở [" << node->token.filename << ":" << node->token.line << ":" << node->token.col << "] " << node->token.getLine() << "\n";
        std::cout << os.str();
        this->throwRuntimeErr(node->token, e.what());
    } catch (Diagnostic& e) {
        throw e.withCallSite(node->token);
    }
    catch (std::runtime_error &e) {
        this->throwRuntimeErr(node->token, e.what());
    }

    return Value(Null{});
}

Value TreeWalker::visit(IndexExpression* node) {
    Value left = evaluate(node->left.get());

    if (Indexable* indexable = toIndexable(left)) {
        Value index = evaluate(node->index.get());
        try {
            return indexable->get(index);
        } catch (const FunctionException& e) {
            throw Diagnostic::RuntimeErr(e.what(), node->token);
        } catch (const std::runtime_error& e) {
            throw Diagnostic::RuntimeErr(e.what(), node->token);
        }
    }
    std::ostringstream os;
    os << "Chỉ có thể truy cập phần tử của Mảng hoặc Object: '";
    os << left;
    os << "' và index: '";
    os << evaluate(node->index.get()) << "'";

    auto diag = Diagnostic::RuntimeErr(os.str(), node->token);
    throw Diagnostic::RuntimeErr(diag.str(), node->token);
}

Value TreeWalker::visit(AssignmentExpression* node) {
    try {
        LValue target = resolveLValue(node->target.get());
        Value value = evaluate(node->value.get());

        target.setter(value);
        return value;
    } catch (const FunctionException& e) {
        throw Diagnostic::RuntimeErr(e.what(), node->token);
    } catch (const std::runtime_error& e) {
        throw Diagnostic::RuntimeErr(e.what(), node->token);
    }
}
Value TreeWalker::visit(TernaryExpression* node) {
    Value condition = evaluate(node->condition.get());
    if (isTruthy(condition)) {
        return evaluate(node->thenBranch.get());
    } else {
        return evaluate(node->elseBranch.get());
    }
    return Value(Null{});
}
Value TreeWalker::visit(PropertyAccess* node) {
    Value object = evaluate(node->object.get());

    if (Indexable* indexable = toIndexable(object)) {
        Value propertyKey = Value(node->property->name);
        try {
            return indexable->get(propertyKey);
        } catch (const FunctionException& e) {
            throw Diagnostic::RuntimeErr(e.what(), node->token);
        } catch (const std::runtime_error& e) {
            throw Diagnostic::RuntimeErr(e.what(), node->token);
        }
    }

    std::ostringstream os;
    os << "Chỉ có thể truy cập thuộc tính của Object: '" << object << "'!";

    auto diag = Diagnostic::RuntimeErr(os.str(), node->token);
    throw Diagnostic::RuntimeErr(diag.str(), node->token);
}
Value TreeWalker::visit(PropertyAssignment* node) {
    Value target = evaluate(node->targetObj.get());

    if (!std::holds_alternative<Object>(target)) {
        std::ostringstream os;
        os << "Chỉ có thể truy cập thuộc tính của Object: '" << target << "'!";

        auto diag = Diagnostic::RuntimeErr(os.str(), node->token);
        throw Diagnostic::RuntimeErr(diag.str(), node->token);
    }

    auto obj = std::get<Object>(target);
    Value value = evaluate(node->value.get());

    const std::string& name = node->property->token.lexeme;

    HashKey key{Value(name)};

    auto it = obj->pairs.find(key);

    if (it != obj->pairs.end()) {
        it->second = value;
        return Value(Null{});
    }

    obj->pairs[key] = value;

    return Value(Null{});
}
Value TreeWalker::visit(ThisExpression* node) {
    return env->find("this");
}
Value TreeWalker::visit(SuperExpression* node) {
    Value thisExpr = env->find("this");
    Instance object = std::get<Instance>(thisExpr);

    MeowScriptClass* superklass = object->klass->superclass.get();
    
    if (superklass == nullptr) {
        throwRuntimeErr(node->token, "Không có lớp cha để gọi 'super'.");
    }

    Function method = nullptr;

    if (node->isCallable) {
        method = superklass->findMethod("init");
    } else if (node->method != nullptr) {
        method = superklass->findMethod(node->method->name);
    }
    if (method == nullptr) {
        throwRuntimeErr(node->token, "Không tìm thấy phương thức trên lớp cha.");
    }

    return std::make_shared<MeowScriptBoundMethod>(object, method);
}

Value TreeWalker::visit(NewExpression* node) {
    Value expr = evaluate(node->expression.get());

    return expr;
}

Value TreeWalker::visit(PrefixUpdateExpression* node) {
    LValue lvalue = resolveLValue(node->operand.get());

    if (!std::holds_alternative<Int>(lvalue.currentValue)) {
        throwRuntimeErr(node->token, "Toán tử ++/-- chỉ dùng cho số nguyên.");
    }
    
    Value newValue = std::get<Int>(lvalue.currentValue) + (node->op == TokenType::OP_INCREMENT ? 1 : -1);
    lvalue.setter(newValue);
    
    return newValue;
}

Value TreeWalker::visit(PostfixUpdateExpression* node) {
    LValue lvalue = resolveLValue(node->operand.get());

    if (!std::holds_alternative<Int>(lvalue.currentValue)) {
        throwRuntimeErr(node->token, "Toán tử ++/-- chỉ dùng cho số nguyên.");
    }

    Value newValue = std::get<Int>(lvalue.currentValue) + (node->op == TokenType::OP_INCREMENT ? 1 : -1);
    lvalue.setter(newValue);

    return lvalue.currentValue;
}

Value TreeWalker::visit(SpreadExpression* node) {
    return Value(Null{});
}
