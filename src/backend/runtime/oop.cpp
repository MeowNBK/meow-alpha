#include "runtime/oop.hpp"
#include "runtime/value.hpp"
#include "runtime/environment.hpp"
#include "runtime/interpreter.hpp"
#include "runtime/function_arity.hpp"
#include "callable/native_callable.hpp"
#include "callable/function_callable.hpp"
#include "diagnostics/meow_exceptions.hpp"

Value MeowScriptClass::call(Interpreter* engine, const std::vector<Value>& args) {
    Instance instance = std::make_shared<MeowScriptInstance>(shared_from_this(), engine);

    Function initializer = nullptr;

    if (methods.count("init")) {
        initializer = methods.at("init");
    }

    if (initializer != nullptr) {
        MeowScriptBoundMethod(instance, initializer).call(engine, args);
    }
    return instance;
}

Function MeowScriptClass::findMethod(const std::string& methodName) const {
    if (methods.count(methodName)) {
        return methods.at(methodName);
    }
    
    if (superclass != nullptr) {
        return superclass->findMethod(methodName);
    }
    
    return nullptr;
}






Arity MeowScriptClass::arity() const {
    Function initializer = nullptr;
    if (methods.count("init")) {
        initializer = methods.at("init");
    }

    if (initializer != nullptr) {
        return initializer->arity();
    }
    return Arity::fixed(0);
}

std::string MeowScriptClass::toString() const {
    return this->name;
}

Value MeowScriptClass::get(const Value& key) {
    if (!std::holds_alternative<String>(key)) {
        throw std::runtime_error("Tên thuộc tính tĩnh phải là một chuỗi.");
    }
    const std::string& name = std::get<String>(key)->str;

    if (name == "__name__") {
        return this->name;
    } else if (name == "__super__") {
        if (this->superclass != nullptr) {
            return this->superclass;
        }
        return Value(Null{});
    }

    if (static_fields.count(name)) {
        return static_fields.at(name);
    }

    return Value(Null{});
}

void MeowScriptClass::set(const Value& key, const Value& value) {
    if (!std::holds_alternative<String>(key)) {
        throw std::runtime_error("Tên thuộc tính tĩnh phải là một chuỗi.");
    }
    const std::string& name = std::get<String>(key)->str;
    
    static_fields[name] = value;
}

Value MeowScriptInstance::get(const Value& key) {





            





            



    Value value = this->fields->get(key);
    if (value != Value(Null{})) {
        return value;
    }
    
    if (std::holds_alternative<String>(key)) {
        const std::string& name = std::get<String>(key)->str;
        if (name == "__class__") {
            return this->klass;
        } else if (name == "__fields__") {
            return this->fields;
        } else if (name == "__instanceof__") {
            Instance self = shared_from_this();
            NativeFnAdvanced instanceof = [self](Interpreter* engine, const std::vector<Value>& args) -> Value {
                if (!std::holds_alternative<Class>(args[0])) {
                    throw std::runtime_error("Hàm __instanceof__ cần đúng 1 tham số là một Class.");
                }
                const Class& targetClass = std::get<Class>(args[0]);
                
                MeowScriptClass* currClass = self->klass.get();
                while (currClass != nullptr) {
                    if (currClass == targetClass.get()) {
                        return true;
                    }
                    currClass = currClass->superclass.get();
                }
                return false;
            };
            return std::make_shared<NativeCallable>(std::make_shared<NativeFunction>("__instanceof__", instanceof, Arity::fixed(1)));
        } else if (name == "__hasmethod__") {
            Instance self = shared_from_this();
            NativeFnAdvanced hasmethod = [self](Interpreter* engine, const std::vector<Value>& args) -> Value {
                if (!std::holds_alternative<String>(args[0])) {
                    throw std::runtime_error("Hàm __hasmethod__ cần 1 tham số là tên phương thức (chuỗi).");
                }
                const std::string& methodName = std::get<String>(args[0])->str;
                return self->klass->findMethod(methodName) != nullptr;
            };
            return std::make_shared<NativeCallable>(std::make_shared<NativeFunction>("__hasmethod__", hasmethod, Arity::fixed(1)));
        } else if (name == "__getmethod__") {
            Instance self = shared_from_this();
            NativeFnAdvanced getmethod = [self](Interpreter* engine, const std::vector<Value>& args) -> Value {
                if (!std::holds_alternative<String>(args[0])) {
                    throw std::runtime_error("Hàm __getmethod__ cần 1 tham số là tên phương thức (chuỗi).");
                }
                const std::string& methodName = std::get<String>(args[0])->str;
                Function method = self->klass->findMethod(methodName);
                if (method) return method;
                return Value(Null{});
            };
            return std::make_shared<NativeCallable>(std::make_shared<NativeFunction>("__getmethod__", getmethod, Arity::fixed(1)));
        }

        Function method = this->klass->findMethod(name);
        if (method != nullptr) {
            return std::make_shared<MeowScriptBoundMethod>(shared_from_this(), method);
        }
    }

    Function getitem = this->klass->findMethod("__getitem__");
    if (getitem != nullptr) {
        return MeowScriptBoundMethod(shared_from_this(), getitem).call(this->engine, {key});
    }
    
    return Value(Null{});
}

void MeowScriptInstance::set(const Value& key, const Value& value) {













    Function setitem = this->klass->findMethod("__setitem__");
    if (setitem != nullptr) {
        MeowScriptBoundMethod(shared_from_this(), setitem).call(this->engine, {key, value});
        return;
    }
    this->fields->set(key, value);
}

std::string MeowScriptInstance::toString() const {
    Function strMethod = this->klass->findMethod("__str__");
    if (strMethod != nullptr) {
        auto self = std::const_pointer_cast<MeowScriptInstance>(shared_from_this());
        MeowScriptBoundMethod boundStr(self, strMethod);
        try {
            Value result = boundStr.call(this->engine, {});
            if (std::holds_alternative<String>(result)) {
                return std::get<String>(result)->str;
            }
        } catch (...) {
            throw;
        }
    }
    return this->klass->toString() + " instance";
}

Value MeowScriptInstance::call(Interpreter* engine, const std::vector<Value>& args) {
    Function callMethod = this->klass->findMethod("__call__");
    if (callMethod != nullptr) {
        return MeowScriptBoundMethod(shared_from_this(), callMethod).call(engine, args);
    }
    throw std::runtime_error("Instance của class '" + klass->name + "' không thể gọi được (thiếu phương thức __call__).");
}

Arity MeowScriptInstance::arity() const {
    Function callMethod = this->klass->findMethod("__call__");
    if (callMethod != nullptr) {
        return callMethod->arity();
    }
    return Arity::fixed(0);
}

std::unique_ptr<Iterator> MeowScriptInstance::makeIterator() {
    return std::make_unique<InstanceIterator>(this);
}

InstanceIterator::InstanceIterator::InstanceIterator(MeowScriptInstance* instance) : isFinished(false) {
    this->engine = instance->engine;
    
    Function iterMethod = instance->klass->findMethod("__iterator__");
    if (iterMethod == nullptr) {
        throw std::runtime_error("Đối tượng này không phải là một iterable.");
    }
    
    this->iteratorObject = std::make_unique<Value>(
        MeowScriptBoundMethod(instance->shared_from_this(), iterMethod).call(this->engine, {})
    );

    this->advance();
}

InstanceIterator::~InstanceIterator() = default;

void InstanceIterator::advance() {
    if (auto inst = std::get_if<Instance>(&*iteratorObject)) {
        Function nextMethod = (*inst)->klass->findMethod("__next__");
        if (nextMethod != nullptr) {
            try {
                this->nextValue = std::make_unique<Value>(
                    MeowScriptBoundMethod(*inst, nextMethod).call(this->engine, {})
                );
                this->isFinished = false;
            } catch (const std::runtime_error& e) {
                this->isFinished = true;
                this->nextValue.reset();
            }
            return;
        }
    }
    this->isFinished = true;
}

bool InstanceIterator::hasNext() const {
    return !this->isFinished;
}

Value InstanceIterator::next() {
    if (isFinished) {
        throw std::runtime_error("Không còn phần tử nào.");
    }
    
    Value current = *this->nextValue;
    
    this->advance();
    
    return current;
}







Value MeowScriptBoundMethod::call(Interpreter* engine, const std::vector<Value>& args) {
    auto userFunction = dynamic_cast<MeowScriptFunction*>(this->function.get());

    if (userFunction) {
        auto executionEnv = std::make_shared<Environment>(userFunction->getEnv());

        executionEnv->define("this", this->instance);

        auto& decl = userFunction->declaration;
        size_t requiredParams = decl->parameters.size();

        for (size_t i = 0; i < requiredParams; ++i) {
            const std::string& paramName = decl->parameters[i]->name;
            executionEnv->define(paramName, args[i]);
        }

        if (decl->restParam) {
            auto restArrayData = std::make_shared<ArrayData>();
            for (size_t i = requiredParams; i < args.size(); ++i) {
                restArrayData->elements.push_back(args[i]);
            }
            executionEnv->define(decl->restParam->name, Value(Array(restArrayData)));
        }

        try {
            engine->exec(decl->body.get(), executionEnv);
        } catch (const ReturnException& returnValue) {
            return returnValue.value;
        }

        return Value(Null{});
    }

    return this->function->call(engine, args);
}

Value MeowScriptBoundMethod::get(const Value& key) {
    if (!std::holds_alternative<String>(key)) {
        throw std::runtime_error("Tên thuộc tính tĩnh phải là một chuỗi.");
    }
    const std::string& name = std::get<String>(key)->str;

    if (name == "__instance__") {
        return this->instance;
    } else if (name == "__function__") {
        return this->function;
    }

    return Value(Null{});
}

void MeowScriptBoundMethod::set(const Value& key, const Value& value) {
    throw std::runtime_error("Không gán thuộc tính được cho bound method.");
}

Arity MeowScriptBoundMethod::arity() const {
    return this->function->arity();
}

std::string MeowScriptBoundMethod::toString() const {
    return "bound_method";
}