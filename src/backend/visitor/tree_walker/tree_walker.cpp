#include "native_lib/standard_lib.hpp"
#include "visitor/tree_walker.hpp"
#include "common/ast.hpp"
#include "diagnostics/diagnostic.hpp"
#include <iostream>
#include <variant>
#include <type_traits>
#include <sstream>

TreeWalker::TreeWalker()
    : env(std::make_shared<Environment>(nullptr)) {
    initCommon();
}

TreeWalker::TreeWalker(std::shared_ptr<Environment> env)
    : env(env) {
    initCommon();
}

TreeWalker::TreeWalker(ModuleManager* manager, SrcFilePtr sourceFile, Value* exp)
    : moduleManager(manager),
        currSrcFile(std::move(sourceFile)),
        currModuleExports(exp),
        env(std::make_shared<Environment>(nullptr)),
        globalEnv(env) {
    initCommon();
}

TreeWalker::TreeWalker(ModuleManager* manager, SrcFilePtr sourceFile, Value* exp, const std::vector<std::string> &a)
    : moduleManager(manager),
        currSrcFile(std::move(sourceFile)),
        currModuleExports(exp),
        env(std::make_shared<Environment>(nullptr)),
        globalEnv(env),
        argv(a) {
    initCommon();
}

void TreeWalker::loadLibrary(std::unique_ptr<NativeLibrary> library) {
    for (const auto& pair : library->contents) {
        env->define(pair.first, pair.second);
    }
}

void TreeWalker::initCommon() {
    if (!opDispatcher) {
        opDispatcher = std::make_unique<OperatorDispatcher>();
    }
    if (!env) {
        env = std::make_shared<Environment>(nullptr);
    }
    loadLibrary(std::make_unique<CoreLib>());
}

Value interpret(Program* program) {
    TreeWalker treeWalker;
    try {
        for (auto& stmt : program->body) {
            treeWalker.evaluate(stmt.get());
        }
    } catch (ReturnException& e) {
        return e.value;
    } catch (MeowScriptException& e) {
        std::cerr << "Lỗi chưa được bắt: " << e.value << "\n";
    } catch (Diagnostic& e) {
        std::cerr << e.str() << "\n";
    }
    return Value(Null{});
}

Value TreeWalker::evaluate(ASTNode* node) {
    if (node == nullptr) {
        return Value(Null{});
    }

    return node->accept(this);
}

Value TreeWalker::exec(ASTNode* node, std::shared_ptr<Environment> local) {
    auto prevEnv = this->env;
    try {
        env = local;
        if (node != nullptr) {
            evaluate(node);
        }
    } catch (...) {
        env = prevEnv;
        throw;
    }

    env = prevEnv;

    return Value(Null{}); 
}

Value TreeWalker::visit(Program* node) {
    try {
        for (auto& stmt : node->body) {
            this->evaluate(stmt.get());
        }
    } catch (ReturnException& e) {
        return e.value;
    } catch (Diagnostic& e) {
        std::cerr << e.str() << "\n";
    }

    return Value(Null{});
}

template <typename T>
struct is_callable_ptr : std::false_type {};

template <typename U>
struct is_callable_ptr<std::shared_ptr<U>> : std::is_base_of<Callable, U> {};

template <typename T>
inline constexpr bool is_callable_ptr_v = is_callable_ptr<T>::value;

Value TreeWalker::call(const Value& callee, const std::vector<Value>& args) {
    return std::visit([this, &callee, &args](auto&& arg) -> Value {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (is_callable_ptr_v<T>) {
            Function callable = arg;
            
            const Arity& arity = callable->arity();
            int argsCount = args.size();

            if (arity.isVariadic) {
                if (argsCount < arity.required) {
                    throw FunctionException("Hàm cần ít nhất " + std::to_string(arity.required) + " tham số.");
                }
            } else {
                if (argsCount < arity.required || argsCount > arity.required + arity.optional) {
                    std::ostringstream os;
                    os << "Hàm cần từ " + std::to_string(arity.required) + " đến " 
                                        + std::to_string(arity.required + arity.optional) + " tham số.";

                    os << " Nhưng lại nhận được '" << argsCount << "' tham số. Các tham số đó là: ";
                    for (const auto& arg : args) {
                        os << arg << " ";
                    }
                    os << ". Và callee là: " << callee << "\n";
                    throw FunctionException(os.str());
                }
            }
            
            return callable->call(this, args);

        } else {
            std::ostringstream os;
            os << "Đối tượng này không thể gọi được: '" << arg << "' với các tham số là: ";
            for (const auto& a : args) {
                os << "'" << a << "'" << " ";
            }
            throw FunctionException(os.str());
        }
    }, callee);
}

void TreeWalker::execBlock(BlockStatement* block, std::shared_ptr<Environment> environment) {
    auto prevEnv = this->env;
    try {
        this->env = environment;

        for (const auto& stmt : block->statements) {
            evaluate(stmt.get());
        }
    } catch (...) {
        this->env = prevEnv;
        throw;
    }

    this->env = prevEnv;
}

LValue TreeWalker::resolveLValue(ASTNode* node) {
    if (auto identifier = dynamic_cast<Identifier*>(node)) {
        return LValue {
            env->find(identifier->name), 
            [this, name = identifier->name](const Value& newValue) {
                this->env->assign(name, newValue);
            }
        };
    }

    if (auto indexExpr = dynamic_cast<IndexExpression*>(node)) {
        Value left = evaluate(indexExpr->left.get());
        
        if (Indexable* indexable = toIndexable(left)) {
            Value index = evaluate(indexExpr->index.get());
            return LValue {
                indexable->get(index),
                [indexable, index](const Value& newValue) {
                    indexable->set(index, newValue);
                }
            };
        }
        throwRuntimeErr(indexExpr->token, "Đối tượng không thể truy cập bằng chỉ số.");
    }

    if (auto propAccess = dynamic_cast<PropertyAccess*>(node)) {
        Value objectValue = evaluate(propAccess->object.get());
        Value key = Value(propAccess->property->name);

        return std::visit([&](auto&& arg) -> LValue {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, Instance>) {
                Instance inst = arg;
                return LValue {
                    inst->fields->get(key),
                    [inst, key](const Value& newValue) {
                        inst->fields->set(key, newValue);
                    }
                };
            }
            else if constexpr (std::is_same_v<T, Object> || std::is_same_v<T, Array>) {
                if (Indexable* indexable = toIndexable(objectValue)) {
                     return LValue {
                        indexable->get(key),
                        [indexable, key](const Value& newValue) {
                            indexable->set(key, newValue);
                        }
                    };
                }
            }
            
            throwRuntimeErr(propAccess->token, "Không thể gán thuộc tính cho kiểu dữ liệu này.");
            return LValue{};

        }, objectValue);
    }

    throwRuntimeErr(node->token, "Biểu thức không hợp lệ ở vế trái của phép gán.");

    return LValue{};
}
