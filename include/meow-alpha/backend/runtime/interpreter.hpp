#pragma once

#include "runtime/environment.hpp"
#include "runtime/value.hpp"
#include <vector>
#include <string>

struct Token;
struct BlockStatement;
struct ASTNode;

class Interpreter {
public:
    virtual ~Interpreter() = default;
    bool isInsideProtocolCall = false;

    virtual Value call(const Value& callee, const std::vector<Value>& args) = 0;
    virtual void execBlock(BlockStatement* block, std::shared_ptr<Environment> environment) = 0;
    virtual Value exec(ASTNode* node, std::shared_ptr<Environment> local) = 0;
    virtual void throwRuntimeErr(const Token& token, const std::string& message) = 0;
    virtual std::shared_ptr<Environment> getCurrEnv() const = 0;
    virtual std::shared_ptr<Environment> getGlobalEnv() const = 0;
    virtual std::vector<std::string> getArgv() const = 0;
}; 