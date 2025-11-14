#pragma once

#include "runtime/operator_dispatcher.hpp"
#include "common/source_file.hpp"
#include "visitor/visitor.hpp"
#include "runtime/interpreter.hpp"
#include "runtime/environment.hpp"
#include "native_lib/standard_lib.hpp"
#include "module/module_manager.hpp"
#include "diagnostics/meow_exceptions.hpp"
#include <stdexcept>
#include <optional>

Value interpret(Program* program);

struct LValue {
    Value currentValue;
    std::function<void(const Value&)> setter;
};

class TreeWalker: Visitor, Interpreter {
private:
    std::shared_ptr<Environment> env;
    std::shared_ptr<Environment> globalEnv;
    std::optional<Value> currentlyCaughtException;

    ModuleManager* moduleManager;
    SrcFilePtr currSrcFile;
    Value* currModuleExports;

    LValue resolveLValue(ASTNode* node);

    std::unique_ptr<OperatorDispatcher> opDispatcher;

    std::vector<std::string> argv;
public:

    TreeWalker();
    TreeWalker(std::shared_ptr<Environment> env);
    TreeWalker(ModuleManager* manager, SrcFilePtr sourceFile, Value* exp);
    TreeWalker(ModuleManager* manager, SrcFilePtr sourceFile, Value* exp, const std::vector<std::string>& a);
    Value evaluate(ASTNode* node);

    bool isModuleContext = false;

    Value exec(ASTNode* node, std::shared_ptr<Environment> newEnv) override;

    Value visit(Program* node) override;

    Value call(const Value &callee, const std::vector<Value>& args) override;
    inline void throwRuntimeErr(const Token& token, const std::string& message) override {
        throw Diagnostic::RuntimeErr(message, token);
    };

    void execBlock(BlockStatement* block, std::shared_ptr<Environment> environment) override;

    inline std::shared_ptr<Environment> getCurrEnv() const override {
        return this->env;
    }

    inline std::shared_ptr<Environment> getGlobalEnv() const override {
        return this->globalEnv;
    }

    inline std::vector<std::string> getArgv() const override {
        return this->argv;
    }

    void loadLibrary(std::unique_ptr<NativeLibrary> library);
    void initCommon();

    inline void setCaughtException(const Value& v) { 
        currentlyCaughtException = v; 
    }
    inline void clearCaughtException() { 
        currentlyCaughtException.reset(); 
    }

    void addToExports(const std::string& name, const Value& value);

    Value visit(IntegerLiteral* node) override;
    Value visit(RealLiteral* node) override;
    Value visit(StringLiteral* node) override;
    Value visit(BooleanLiteral* node) override;
    Value visit(NullLiteral* node) override;
    Value visit(ArrayLiteral* node) override;
    Value visit(ObjectLiteral* node) override;
    Value visit(FunctionLiteral* node) override;
    Value visit(TemplateLiteral* node) override;

    Value visit(Identifier* node) override;
    Value visit(BinaryExpression* node) override;
    Value visit(UnaryExpression* node) override;
    Value visit(CallExpression* node) override;
    Value visit(IndexExpression* node) override;
    Value visit(AssignmentExpression* node) override;
    Value visit(TernaryExpression* node) override;
    Value visit(PropertyAccess* node) override;
    Value visit(PropertyAssignment* node) override;
    Value visit(ThisExpression* node) override;
    Value visit(SuperExpression* node) override;
    Value visit(NewExpression* node) override;
    Value visit(PrefixUpdateExpression* node) override;
    Value visit(PostfixUpdateExpression* node) override;
    Value visit(SpreadExpression* node) override;

    Value visit(LetStatement* node) override;
    Value visit(ReturnStatement* node) override;
    Value visit(BreakStatement* node) override;
    Value visit(ContinueStatement* node) override;
    Value visit(ThrowStatement* node) override;
    Value visit(IfStatement* node) override;
    Value visit(WhileStatement* node) override;
    Value visit(ForStatement* node) override;
    Value visit(ForInStatement* node) override;
    Value visit(BlockStatement* node) override;
    Value visit(ClassStatement* node) override;
    Value visit(ImportStatement* node) override;
    Value visit(ExportStatement* node) override;
    Value visit(TryStatement* node) override;
    Value visit(ExpressionStatement* node) override;
    Value visit(LogStatement* node) override;
    Value visit(SwitchCase* node) override;
    Value visit(SwitchStatement* node) override;
    Value visit(DoWhileStatement* node) override;
};