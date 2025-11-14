#pragma once

#include "interface/callable.hpp"
#include "runtime/environment.hpp"
#include "common/ast.hpp"
#include "visitor/tree_walker.hpp"

class MeowScriptFunction: public Callable {
public:
    FunctionLiteral* declaration;
    std::shared_ptr<Environment> closure;

    MeowScriptFunction(FunctionLiteral* decl, std::shared_ptr<Environment> cl);
    Value call(Interpreter* engine, const std::vector<Value>& args) override;

    Arity arity() const override;

    std::shared_ptr<Environment> getEnv() override;
};