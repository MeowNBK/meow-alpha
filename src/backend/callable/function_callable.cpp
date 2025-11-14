#include "callable/function_callable.hpp"

MeowScriptFunction::MeowScriptFunction(FunctionLiteral* decl, std::shared_ptr<Environment> cl):
    declaration(decl), closure(cl) {}
Value MeowScriptFunction::call(Interpreter* engine, const std::vector<Value>& args) {
    auto localEnv = std::make_shared<Environment>(this->closure);

    size_t requiredParams = declaration->parameters.size();

    for (size_t i = 0; i < requiredParams; ++i) {
        const std::string& paramName = declaration->parameters[i]->name;

        localEnv->define(paramName, args[i]);
    }

    if (declaration->restParam) {
        auto restArrayData = std::make_shared<ArrayData>();

        for (size_t i = requiredParams; i < args.size(); ++i) {
            restArrayData->elements.push_back(args[i]);
        }

        localEnv->define(declaration->restParam->name, Value(Array(restArrayData)));
    }

    try {
        engine->exec(declaration->body.get(), localEnv);
    } catch (ReturnException& e) {
        return e.value;
    }

    return Value(Null{});
}

Arity MeowScriptFunction::arity() const {

    size_t requiredParams = declaration->parameters.size();

    if (declaration->restParam) {
        return Arity::atLeast(static_cast<int>(requiredParams));
    } else {
        return Arity::fixed(static_cast<int>(requiredParams));
    }
}

std::shared_ptr<Environment> MeowScriptFunction::getEnv() {
    return this->closure;
}