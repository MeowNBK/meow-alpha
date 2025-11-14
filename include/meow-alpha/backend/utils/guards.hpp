#pragma once

#include "runtime/environment.hpp"
#include "visitor/tree_walker.hpp"
#include <memory>

class EnvGuard {
public:
    EnvGuard(std::shared_ptr<Environment>& envToManage): managedEnv(envToManage), originalEnv(envToManage) {
        managedEnv = std::make_shared<Environment>(originalEnv);
    }

    ~EnvGuard() {
        managedEnv = originalEnv;
    }

    EnvGuard(const EnvGuard&) = delete;
    EnvGuard& operator=(const EnvGuard&) = delete;

private:
    std::shared_ptr<Environment>& managedEnv; 
    
    std::shared_ptr<Environment> originalEnv;
};

class CaughtExceptionGuard {
public:
    CaughtExceptionGuard(TreeWalker* w, const Value& value): walker(w) {
        walker->setCaughtException(value);
    }

    ~CaughtExceptionGuard() {
        walker->clearCaughtException();
    }

    CaughtExceptionGuard(const CaughtExceptionGuard&) = delete;
    CaughtExceptionGuard& operator=(const CaughtExceptionGuard&) = delete;

private:
    TreeWalker* walker;
};
