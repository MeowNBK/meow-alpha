#include "visitor/tree_walker.hpp"
#include "utils/guards.hpp"
#include "common/ast.hpp"
#include "diagnostics/diagnostic.hpp"
#include "callable/function_callable.hpp"
#include <iostream>
#include <sstream>

Value TreeWalker::visit(LetStatement* node) {
    Value value = evaluate(node->value.get());
    env->define(node->name->name, value, node->isConstant);

    return value;
}
Value TreeWalker::visit(ReturnStatement* node) {

    if (node->value != nullptr) {
        throw ReturnException(node->value->accept(this));
    }

    throw ReturnException(Value(Null{}));

    return Value(Null{});
}
Value TreeWalker::visit(BreakStatement* node) {
    throw BreakException();

    return Value(Null{});
}
Value TreeWalker::visit(ContinueStatement* node) {
    throw ContinueException();
    return Value(Null{});
}
Value TreeWalker::visit(ThrowStatement* node) {
    if (node->argument) {
        Value argument = evaluate(node->argument.get());
        throw MeowScriptException(argument);
        return argument;
    }

    if (!currentlyCaughtException.has_value()) {
        throwRuntimeErr(node->token, "Bạn dùng lệnh 'throw' ở đâu thế này, không ở trong khối 'catch' à?");
    }

    throw MeowScriptException(currentlyCaughtException.value());
}

Value TreeWalker::visit(IfStatement* node) {
    EnvGuard guard(this->env);

    Value condition = evaluate(node->condition.get());

    if (isTruthy(condition)) {
        evaluate(node->thenBranch.get());
    } else if (node->elseBranch != nullptr) {
        evaluate(node->elseBranch.get());
    }

    return Value(Null{});
}
Value TreeWalker::visit(WhileStatement* node) {
    EnvGuard guard(this->env);
    while (isTruthy(evaluate(node->condition.get()))) {
        try {
            evaluate(node->body.get());
        } catch (const BreakException&) {
            break;
        } catch (const ContinueException&) {
            continue;
        }
    }

    return Value(Null{});
}
Value TreeWalker::visit(ForStatement* node) {
    EnvGuard guard(this->env);

    if (node->init != nullptr) {
        evaluate(node->init.get());
    }

    while (true) {
        if (node->condition != nullptr) {
            Value condition = evaluate(node->condition.get());
            if (!isTruthy(condition)) {
                break;
            }
        }

        try {
            evaluate(node->body.get());
        } catch (const BreakException&) {
            break;
        } catch (const ContinueException&) {
            if (node-> update != nullptr) {
                evaluate(node->update.get());
                continue;
            }
        }

        if (node->update != nullptr) {
            evaluate(node->update.get());
        }
    }

    return Value(Null{});
}
Value TreeWalker::visit(ForInStatement* node) {
    EnvGuard guard(this->env);

    Value collection = evaluate(node->collection.get());

    if (Iterable* iterable = toIterable(collection)) {
        auto iterator = iterable->makeIterator();
        while (iterator->hasNext()) {

            env->define(node->variable->name, iterator->next());

            try {
                evaluate(node->body.get());
            } catch (const BreakException&) {
                break;
            } catch (const ContinueException&) {
                continue;
            }
        }
    } else {
        throwRuntimeErr(node->token, "Kiểu dữ liệu này không thể duyệt qua.");
    }

    return Value(Null{});
}
Value TreeWalker::visit(BlockStatement* node) {
    EnvGuard guard(this->env);

    for (const auto& stmt : node->statements) {
        evaluate(stmt.get());
    }

    return Value(Null{});
}
Value TreeWalker::visit(ClassStatement* node) {
    Class superklass = nullptr;
    if (node->superclass != nullptr) {
        Value superVal = env->find(node->superclass->name);
        if (auto cls = std::get_if<Class>(&superVal)) {
            superklass = *cls;
        } else {
            throwRuntimeErr(node->superclass->token, "Class cha không phải là một class.");
        }
    }

    Class klass = std::make_shared<MeowScriptClass>(node->name->name, superklass);

    env->define(node->name->name, klass);

    for (const auto& methodStmt : node->methods) {
        auto letStmt = static_cast<LetStatement*>(methodStmt.get());
        auto funcLiteral = static_cast<FunctionLiteral*>(letStmt->value.get());
        
        Function function = std::make_shared<MeowScriptFunction>(funcLiteral, env); 
        
        klass->methods[letStmt->name->name] = function;
    }

    for (const auto& field : node->static_fields) {
        if (auto letStmt = dynamic_cast<LetStatement*>(field.get())) {
            auto staticValue = evaluate(letStmt->value.get());
            klass->static_fields[letStmt->name->name] = staticValue;
        } else if (auto classStmt = dynamic_cast<ClassStatement*>(field.get())) {
            auto staticValue = evaluate(classStmt);
            klass->static_fields[classStmt->name->name] = staticValue;
        }
    }

    return Value(Null{});
}

Value TreeWalker::visit(ImportStatement* node) {
    Value pathValue = evaluate(node->path.get());
    if (!std::holds_alternative<String>(pathValue)) {
        throwRuntimeErr(node->token, "Đường dẫn phải là chuỗi chứ bạn!");
    }
    std::string importPath = std::get<String>(pathValue)->str;
    std::string currentPath = currSrcFile->name();

    Value exportsValue = moduleManager->load(currentPath, importPath);
    auto exportsObj = std::get<Object>(exportsValue);

    if (node->namespaceImport) {
        env->define(node->namespaceImport->name, exportsValue);
    } else if (!node->namedImports.empty()) {
        for (const auto& specifier : node->namedImports) {
            std::string name = specifier->name;
            auto it = exportsObj->pairs.find(HashKey{Value(name)});
            if (it == exportsObj->pairs.end()) {
                throwRuntimeErr(specifier->token, "Module không export '" + name + "'.");
            }
            env->define(name, it->second);
        }
    } else if (node->importAll) {
        for (const auto& pair : exportsObj->pairs) {
            if (std::holds_alternative<String>(pair.first.value)) {
                std::string name = std::get<String>(pair.first.value)->str;
                env->define(name, pair.second);
            }
        }
    }
    return Value(Null{});
}

void TreeWalker::addToExports(const std::string& name, const Value& value) {
    if (currModuleExports && std::holds_alternative<Object>(*currModuleExports)) {
        auto exportsObj = std::get<Object>(*currModuleExports);
        exportsObj->pairs[HashKey{Value(name)}] = value;
    }
}

Value TreeWalker::visit(ExportStatement* node) {
    if (!isModuleContext) {
        throwRuntimeErr(node->token, "Không thể dùng 'export' trong file chính hoặc eval ngoài module.");
    }
    if (!node->specifiers.empty()) {
        for (const auto& specifier : node->specifiers) {
            std::string name = specifier->name;
            Value value = env->find(name);
            addToExports(name, value);
        }
        return Value(Null{});
    }

    if (node->declaration) {
        evaluate(node->declaration.get());

        std::string name;

        if (auto letStmt = dynamic_cast<LetStatement*>(node->declaration.get())) {
            name = letStmt->name->name;
        } else if (auto classStmt = dynamic_cast<ClassStatement*>(node->declaration.get())) {
            name = classStmt->name->name;
        }

        if (!name.empty()) {
            Value value = env->find(name);
            addToExports(name, value);
        }
        else {
            throwRuntimeErr(node->token, "Câu lệnh export này không được hỗ trợ đâu.");
        }
    }
    
    return Value(Null{});
}

Value TreeWalker::visit(TryStatement* node) {
    try {
        evaluate(node->tryBlock.get());
    } catch (MeowScriptException& e) {
        CaughtExceptionGuard guard(this, e.value);

        auto catchEnv = std::make_shared<Environment>(this->env);

        catchEnv->define(node->catchVariable->name, e.value, false);

        this->exec(node->catchBlock.get(), catchEnv);
    }

    return Value(Null{});
}
Value TreeWalker::visit(ExpressionStatement* node) {
    Value expr = evaluate(node->expression.get());
    return expr;
}

Value TreeWalker::visit(LogStatement* node) {
    Value value = evaluate(node->expression.get());

    std::cout << value;

    return Value(Null{});
}

Value TreeWalker::visit(SwitchCase* node) {
    return Value(Null{});
}
Value TreeWalker::visit(SwitchStatement* node) {
    Value value = evaluate(node->value.get());
    
    int startIdx = -1;
    int defaultIdx = -1;

    for (int i = 0; i < node->cases.size(); ++i) {
        const auto& currentCase = node->cases[i];
        
        if (currentCase->value) {
            if (value == evaluate(currentCase->value.get())) {
                startIdx = i;
                break;
            }
        } else {
            defaultIdx = i;
        }
    }

    if (startIdx == -1 && defaultIdx != -1) {
        startIdx = defaultIdx;
    }

    if (startIdx != -1) {
        for (int i = startIdx; i < node->cases.size(); ++i) {
            const auto& currentCase = node->cases[i];
            try {
                for (const auto& stmt : currentCase->statements) {
                    evaluate(stmt.get());
                }
            } catch (const BreakException&) {
                return Value(Null{}); 
            }
        }
    }

    return Value(Null{});
}

Value TreeWalker::visit(DoWhileStatement* node) {
    do {
        try {
            evaluate(node->body.get());
        } catch (const ContinueException&) {
            continue;
        } catch (const BreakException&) {
            break;
        }
    } while (isTruthy(evaluate(node->condition.get())));

    return Value(Null{});
}