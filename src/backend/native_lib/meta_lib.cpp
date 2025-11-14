#include "native_lib/standard_lib.hpp"
#include "diagnostics/meow_exceptions.hpp"

#include "runtime/interpreter.hpp"
#include "common/source_file.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "visitor/tree_walker.hpp"

#include "visitor/print_visitor.hpp"
#include "visitor/ast_builder.hpp"

#include "module/module_manager.hpp"


Value getEnv(Interpreter* engine, Arguments args) {

    std::shared_ptr<Environment> currentEnv = engine->getCurrEnv();
    
    auto allVariables = currentEnv->getAllVariables();

    auto envObject = std::make_shared<ObjectData>();
    for (const auto& pair : allVariables) {
        const std::string& varName = pair.first;
        const Variable& variable = pair.second;

        auto varObj = std::make_shared<ObjectData>();
        varObj->pairs[HashKey{Value("value")}] = variable.value;
        varObj->pairs[HashKey{Value("isConstant")}] = Value(variable.isConstant);

        envObject->pairs[HashKey{Value(varName)}] = Value(Object(varObj));
    }
    
    return Value(Object(envObject));
}

Value tokenToValue(const Token& token) {
    auto objData = std::make_shared<ObjectData>();
    objData->pairs.reserve(5); 

    objData->pairs[HashKey{"type"}]   = Value(std::string(tokenTypeToString(token.type)));
    objData->pairs[HashKey{"lexeme"}] = Value(token.lexeme);
    objData->pairs[HashKey{"line"}]   = Value(static_cast<Int>(token.line));
    objData->pairs[HashKey{"col"}]    = Value(static_cast<Int>(token.col));
    objData->pairs[HashKey{"file"}]   = Value(token.filename);

    return Value(objData);
}

Token valueToToken(const Value& value) {
    if (!std::holds_alternative<Object>(value)) {
        throw std::runtime_error("Giá trị phải là một Object để chuyển thành Token!");
    }

    Object obj = std::get<Object>(value);
    
    HashKey typeKey{"type"};
    HashKey lexemeKey{"lexeme"};
    HashKey lineKey{"line"};
    HashKey colKey{"col"};
    HashKey fileKey{"file"};

    auto typeIt = obj->pairs.find(typeKey);
    auto lexemeIt = obj->pairs.find(lexemeKey);
    auto lineIt = obj->pairs.find(lineKey);
    auto colIt = obj->pairs.find(colKey);
    auto fileIt = obj->pairs.find(fileKey);

    if (typeIt == obj->pairs.end() || lexemeIt == obj->pairs.end() || lineIt == obj->pairs.end() || colIt == obj->pairs.end() || fileIt == obj->pairs.end()) {
        throw std::runtime_error("Object thiếu key 'type', 'lexeme', 'line', 'col' hoặc 'file' để tạo Token!");
    }

    Str typeStr;
    if (std::holds_alternative<String>(typeIt->second)) {
        typeStr = std::get<String>(typeIt->second)->str;
    } else {
        throw std::runtime_error("Giá trị của key 'type' phải là một chuỗi!");
    }

    Str lexemeStr;

    if (std::holds_alternative<String>(lexemeIt->second)) {
        lexemeStr = std::get<String>(lexemeIt->second)->str;
    } else {
        throw std::runtime_error("Giá trị của key 'lexeme' phải là một chuỗi!");
    }

    Int lineNum;
    if (std::holds_alternative<Int>(lineIt->second)) {
        lineNum = std::get<Int>(lineIt->second);
    } else {
        throw std::runtime_error("Giá trị của key 'line' phải là một số nguyên!");
    }

    Int colNum;
    if (std::holds_alternative<Int>(colIt->second)) {
        colNum = std::get<Int>(colIt->second);
    } else {
        throw std::runtime_error("Giá trị của key 'col' phải là một số nguyên!");
    }

    Str filenameStr;
    if (std::holds_alternative<String>(fileIt->second)) {
        filenameStr = std::get<String>(fileIt->second)->str;
    } else {
        throw std::runtime_error("Giá trị của key 'file' phải là một chuỗi!");
    }
    
    TokenType type = stringToTokenType(typeStr);

    SrcFilePtr srcFile = std::make_shared<SourceFile>("", filenameStr);

    return Token(type, lexemeStr, filenameStr, static_cast<size_t>(lineNum), static_cast<size_t>(colNum), std::move(srcFile));
}
Value tokenize(Arguments args) {
    if (args.empty() || !std::holds_alternative<String>(args[0])) {
        throw FunctionException("Hàm 'tokenize' cần 1 tham số là chuỗi mã nguồn.");
    }
    const std::string& source = std::get<String>(args[0])->str;

    SrcFilePtr srcFile = std::make_unique<SourceFile>(source, "[lex string]");
    Lexer lexer(srcFile);
    std::vector<Token> tokens = lexer.tokenize();

    auto array = std::make_shared<ArrayData>();
    array->elements.reserve(tokens.size()); 

    for (const auto& token : tokens) {
        array->elements.push_back(tokenToValue(token));
    }

    return Value(array);
}

Value parse(Interpreter* engine, Arguments args) {
    if (args.empty() || !std::holds_alternative<Array>(args[0])) {
        throw FunctionException("Hàm 'parse' cần 1 tham số là mảng tokens.");
    }
    auto tokenArray = std::get<Array>(args[0]);

    std::vector<Token> tokens;
    tokens.reserve(tokenArray->elements.size());
    for (const auto& v : tokenArray->elements) {
        tokens.push_back(valueToToken(v));
    }

    Parser parser(tokens);
    std::unique_ptr<Program> program;
    try {
        program = parser.parseProgram();
    } catch (const Diagnostic& e) {
        throw FunctionException("Lỗi khi parse tokens: " + e.str());
    }

    PrintVisitor printer;
    Value astAsValue = printer.visit(program.get());

    if (!std::holds_alternative<Object>(astAsValue)) {
        throw FunctionException("PrintVisitor không trả về Object.");
    }

    return astAsValue;
}

Value execute(Interpreter* engine, Arguments args) {
    if (args.empty() || !std::holds_alternative<Object>(args[0])) {
        throw FunctionException("Hàm 'execute' cần 1 tham số là Object đại diện cho AST.");
    }
    const Value& astAsValue = args[0];

    ASTBuilder astBuilder;
    ASTNodePtr astNode;
    try {
        astNode = astBuilder.buildFromObject(astAsValue);
    } catch (const std::exception& e) {
        throw FunctionException("Lỗi khi xây dựng lại AST: " + std::string(e.what()));
    }
    
    Program* program = dynamic_cast<Program*>(astNode.get());
    if (!program) {
        throw FunctionException("AST không phải là một Program node hợp lệ.");
    }

    TreeWalker treeWalker;
    Value result;
    try {
        result = treeWalker.visit(program);
    } catch (const Diagnostic& e) {
        throw FunctionException("Lỗi trong quá trình thực thi: " + e.str());
    }

    return result;
}

Value compile(Arguments args) {
    if (args.empty() || !std::holds_alternative<String>(args[0])) {
        throw FunctionException("Hàm 'compile' cần 1 tham số là chuỗi mã nguồn.");
    }
    const std::string& source = std::get<String>(args[0])->str;

    SrcFilePtr srcFile = std::make_unique<SourceFile>(source, "[compile string]");
    Lexer lexer(srcFile);
    std::vector<Token> tokens = lexer.tokenize();

    Parser parser(tokens);
    std::unique_ptr<Program> program;
    try {
        program = parser.parseProgram();
    } catch (const Diagnostic& e) {
        throw FunctionException("Lỗi khi parse mã nguồn: " + e.str());
    }

    PrintVisitor printer;
    Value astAsValue = printer.visit(program.get());

    if (!std::holds_alternative<Object>(astAsValue)) {
        throw FunctionException("PrintVisitor không trả về Object.");
    }

    return astAsValue;
}













Value eval(Interpreter* engine, Arguments args) {
    if (args.empty() || !std::holds_alternative<String>(args[0])) {
        throw FunctionException("Hàm 'eval' cần 1 tham số là chuỗi code.");
    }
    bool useSandbox = true;

    size_t argsCount = args.size();
    if (argsCount > 1 && std::holds_alternative<Object>(args.back())) {
        Object opts = std::get<Object>(args.back());
        bool isOptionObject = false;

        auto it_sb = opts->pairs.find(HashKey{Value("sandbox")});
        if (it_sb != opts->pairs.end() && std::holds_alternative<Bool>(it_sb->second)) {
            useSandbox = std::get<Bool>(it_sb->second);
            isOptionObject = true;
        }

        if (isOptionObject) {
            argsCount--;
        }
    }

    const std::string& codeToEval = std::get<String>(args[0])->str;

    std::shared_ptr<Environment> targetEnv = useSandbox ? std::make_shared<Environment>(engine->getCurrEnv()): engine->getGlobalEnv();

    TreeWalker walker(targetEnv);

    try {
        SrcFilePtr srcFile = std::make_unique<SourceFile>(codeToEval, "[eval'd code]");
        Lexer lexer(srcFile);
        auto tokens  = lexer.tokenize();
        Parser parser(tokens);
        auto program = parser.parseProgram();
        return walker.visit(program.get());
    } catch (const Diagnostic& e) {
        throw FunctionException("Lỗi trong code eval: " + e.str());
    }
}


MetaLib::MetaLib() {

    registerFn("getEnv", getEnv, 0);
    registerFn("tokenize", tokenize, 1);
    registerFn("parse", parse, 1);
    registerFn("execute", execute, 1);
    registerFn("compile", compile, 1);
    registerFn("eval", eval, Arity::range(1, 1));
}