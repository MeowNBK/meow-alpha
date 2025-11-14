#pragma once

#include "runtime/value.hpp"
#include "diagnostics/diagnostic.hpp"
#include <memory>
#include <unordered_map>
#include <stdexcept>
#include <iostream>

class Value;

struct Variable {
    Value value;
    bool isConstant;
};

class Environment {
private:
    std::unordered_map<std::string, Variable> variables;
    std::shared_ptr<Environment> outer;
public:
    Environment(std::shared_ptr<Environment> parent): outer(std::move(parent)) {}

    void define(const std::string& name, const Value& value, bool isConstant = false) {
        variables[name] = {value, isConstant};
    }

    void assign(const std::string& name, const Value& value) {
        auto it = variables.find(name);

        if (it != variables.end()) {
            if (it->second.isConstant) {
                throw std::runtime_error("Không thể gán cho biến '" + name +  "' vì nó là hằng số!");
            }
            it->second.value = value;
            return;
        }

        if (outer) {
            outer->assign(name, value);
            return;
        }

        variables[name] = {value, false};
    }

    Value find(const std::string& name) const {
        auto it = variables.find(name);

        if (it != variables.end()) {
            return it->second.value;
        }

        if (outer) {
            return outer->find(name);
        }

        return Value(Null{});
    }

    void setConst(const std::string& name) {
        auto it = variables.find(name);

        if (it != variables.end()) {
            it->second.isConstant = true;
            return;
        }

        if (outer) {
            outer->setConst(name);
            return;
        }
    }

    void unsetConst(const std::string& name) {
        auto it = variables.find(name);

        if (it != variables.end()) {
            it->second.isConstant = false;
            return;
        }

        if (outer) {
            outer->unsetConst(name);
            return;
        }
    }

    std::unordered_map<std::string, Variable> getAllVariables() const {
        std::unordered_map<std::string, Variable> allVars;
        
        if (outer) {
            allVars = outer->getAllVariables();
        }
        
        for (const auto& pair : variables) {
            allVars[pair.first] = pair.second;
        }
        
        return allVars;
    }
};