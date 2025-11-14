#include "parser/parser.hpp"

using enum TokenType;
using enum NodeType;

StmtPtr Parser::letDeclaration(Token token, bool isConstant = false) {
    auto identifier = std::make_unique<Identifier>(consume(IDENTIFIER, "Expected identifier"));
    ExprPtr value = nullptr;
    if (match({OP_ASSIGN})) {
        value = expression();
    }

    consume(PUNCT_SEMICOLON, "Thiếu dấu phẩy ';' sau một câu định nghĩa let!");

    return std::make_unique<LetStatement>(token, std::move(identifier), std::move(value), isConstant);
}

StmtPtr Parser::functionDeclaration(Token token) {
    auto identifier = std::make_unique<Identifier>(consume(IDENTIFIER, "Cần một cái tên cho hàm. Bạn quên rồi à?"));

    ExprPtr literal = parseFunctionTail(token);

    return std::make_unique<LetStatement>(token, std::move(identifier), std::move(literal));
}

StmtPtr Parser::classDeclaration(Token token) {
    IdenPtr name = std::make_unique<Identifier>(consume(IDENTIFIER, "Khi định nghĩa class thì chắc chắn là cần một cái tên!"));
    IdenPtr superclass = nullptr;

    if (match({PUNCT_COLON})) {
        superclass = std::make_unique<Identifier>(consume(IDENTIFIER, "Bạn dùng dấu hai chấm ':' nhưng lại không ghi tên class cha ngay sau đấy!"));
    }

    consume(PUNCT_LBRACE, "Class thì không phải if/else hay while đâu nên thêm dấu ngoặc nhọn '{' cho thân class đi!");

    std::vector<StmtPtr> methods;
    std::vector<StmtPtr> static_fields;

    while (!check(PUNCT_RBRACE) && !isAtEnd()) {
        if (match({KEYWORD_STATIC})) {
            if (check(KEYWORD_LET) || check(KEYWORD_FUNCTION) || check(KEYWORD_CLASS)) {
                static_fields.push_back(declaration());
            } else {
                throw Diagnostic::ParseErr("Sau 'static' trong một class phải là các định nghĩa!", peek());
            }
        } else {
            methods.push_back(functionDeclaration(advance()));
        }
    }
    consume(PUNCT_RBRACE, "Chịu rồi bạn, thiếu '}' sau class");

    return make_unique<ClassStatement>(token, std::move(name), std::move(superclass), std::move(methods), std::move(static_fields));
}

StmtPtr Parser::ifStatement(Token token) {
    auto condition = expression();

    auto thenBranch = statement();

    StmtPtr elseBranch = nullptr;

    if (match({KEYWORD_ELSE})) {
        elseBranch = statement();
    }
    return std::make_unique<IfStatement>(token, std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

StmtPtr Parser::whileStatement(Token token) {
    auto condition = expression();
    StmtPtr body = statement();

    return std::make_unique<WhileStatement>(token, std::move(condition), std::move(body));
}

StmtPtr Parser::forStatement(Token token) {
    bool hasParen = match({PUNCT_LPAREN});

    bool isForIn = (peek().type == IDENTIFIER && (next().type == KEYWORD_IN || next().type == PUNCT_COLON));

    if (isForIn) {
        IdenPtr variable = std::make_unique<Identifier>(advance());
        if (!match({KEYWORD_IN, PUNCT_COLON})) {
            throw Diagnostic::ParseErr("Thiếu 'in' hoặc ':' khi lặp qua", peek());
        }
        ExprPtr collection = expression();
        if (hasParen) {
            consume(PUNCT_RPAREN, "Cần ')' để kết thúc for-in!");
        } else {
            match({PUNCT_RPAREN});
        }
        StmtPtr body = statement();
        return std::make_unique<ForInStatement>(token, std::move(variable), std::move(collection), std::move(body));
    }

    StmtPtr init = nullptr;
    if (!match({PUNCT_SEMICOLON})) {
        init = declaration();
    }
    ExprPtr condition = nullptr;
    if (!check(PUNCT_SEMICOLON)) {
        condition = expression();
    }
    consume(PUNCT_SEMICOLON, "Cần ';' sau điều kiện!");
    ExprPtr update = nullptr;
    if (!check(PUNCT_RPAREN)) {
        update = expression();
    }
    if (hasParen) {
        consume(PUNCT_RPAREN, "Cần ')' để kết thúc for clauses!");
    } else {
        match({PUNCT_RPAREN});
    }
    StmtPtr body = statement();
    return std::make_unique<ForStatement>(token,std::move(init), std::move(condition), std::move(update), std::move(body));
}

StmtPtr Parser::returnStatement(Token token) {
    ExprPtr value = nullptr;

    if (!check(PUNCT_SEMICOLON)) {
        value = expression();
    }

    consume(PUNCT_SEMICOLON, "Cần một dấu chấm phẩy ';' ở đây nhá bạn!");

    return std::make_unique<ReturnStatement>(token, std::move(value));
}

StmtPtr Parser::breakStatement(Token token) {
    consume(PUNCT_SEMICOLON, "Cần một dấu chấm phẩy ';' ở đây nhá bạn!");
    return std::make_unique<BreakStatement>(token);
}

StmtPtr Parser::continueStatement(Token token) {
    consume(PUNCT_SEMICOLON, "Cần một dấu chấm phẩy ';' ở đây nhá bạn!");
    return std::make_unique<ContinueStatement>(token);
}

StmtPtr Parser::blockStatement(Token token) {
    auto block = std::make_unique<BlockStatement>(token);

    while (!check(PUNCT_RBRACE) && !isAtEnd()) {
        auto decl = declaration();
        if (decl) {
            block->statements.push_back(std::move(decl));
        }
    }

    consume(PUNCT_RBRACE, "Expected '}");
    return block;
}

StmtPtr Parser::throwStatement(Token token) {
    ExprPtr args;
    if (!check(PUNCT_SEMICOLON)) {
        args = expression();
    }

    consume(PUNCT_SEMICOLON, "Cần một dấu chấm phẩy ';' ở đây nhá bạn!");

    return std::make_unique<ThrowStatement>(token, std::move(args));
}

StmtPtr Parser::tryStatement(Token token) {
    StmtPtr tryBlock = blockStatement(consume(PUNCT_LBRACE, "Try không nhận gì khác ngoài một block '{}' sau nó đâu! Nên là thêm dấu ngoặc kép '{' vào nhá bạn."));

    consume(KEYWORD_CATCH, "Có 'try' mà không có 'catch'?");
    consume(PUNCT_LPAREN, "Thiếu dấu ngoặc đơn trái '(' rồi");
    IdenPtr catchVar = std::make_unique<Identifier>(consume(IDENTIFIER, "Bạn cần bắt thứ gì? Tên nó là gì?"));
    consume(PUNCT_RPAREN, "Thiếu dầu ngoặc đơn phải ')'.");

    StmtPtr catchBlock = statement();

    return make_unique<TryStatement>(token, std::move(tryBlock), std::move(catchVar), std::move(catchBlock));
}

StmtPtr Parser::importStatement(Token token) {
    std::vector<IdenPtr> namedImports;
    IdenPtr namespaceImport = nullptr;
    ExprPtr path = nullptr;
    bool importAll = false;

    if (match({PUNCT_LBRACE})) {
        while (!check(PUNCT_RBRACE) && !isAtEnd()) {
            do {
                namedImports.push_back(std::make_unique<Identifier>(consume(IDENTIFIER, "Cần tên định danh trong danh sách import.")));
            } while (match({PUNCT_COMMA}));
        }
        consume(PUNCT_RBRACE, "Thiếu ngoặc nhọn '}' để đóng danh sách import.");
        consume(KEYWORD_FROM, "Thiếu từ khóa 'from' sau danh sách import.");
        path = expression();

    } else if (match({OP_MULTIPLY})) {
        consume(KEYWORD_AS, "Thiếu từ khóa 'as' sau '*'.");
        namespaceImport = std::make_unique<Identifier>(consume(IDENTIFIER, "Cần một tên namespace sau 'as'."));
        consume(KEYWORD_FROM, "Thiếu từ khóa 'from' sau tên namespace.");
        path = expression();

        if (match({KEYWORD_AS})) {
            namespaceImport = std::make_unique<Identifier>(consume(IDENTIFIER, "Cần một tên namespace sau 'as'."));
            consume(KEYWORD_FROM, "Thiếu từ khóa 'from' sau tên namespace.");
            path = expression();
        } else if (match({KEYWORD_FROM})) {
            path = expression();
            importAll = true;
        }
        
    } else {
        path = expression();
        importAll = true;
    }

    consume(PUNCT_SEMICOLON, "Thiếu dấu ';' cuối câu lệnh import.");
    
    return std::make_unique<ImportStatement>(token, std::move(path), std::move(namedImports), std::move(namespaceImport), importAll);
}

StmtPtr Parser::exportStatement(Token token) {
    StmtPtr decl;
    if (check(KEYWORD_LET) || check(KEYWORD_CONST) || check(KEYWORD_FUNCTION) || check(KEYWORD_CLASS)) {
        decl = declaration();
        return std::make_unique<ExportStatement>(token, std::move(decl), std::vector<IdenPtr>{});
    } else if (match({PUNCT_LBRACE})) {
        std::vector<IdenPtr> specifiers;

        while (!check(PUNCT_RBRACE) && !isAtEnd()) {
            do {
                specifiers.push_back(std::make_unique<Identifier>(consume(IDENTIFIER, "Cần tên biến trong danh sách export")));
            } while (match({PUNCT_COMMA}));
        }
        consume(PUNCT_RBRACE, "Thiếu dấu ngoặc nhọn '}' sau danh sách export");
        consume(PUNCT_SEMICOLON, "Thiếu dấu chấm phẩy ';' sau câu lệnh.");

        return std::make_unique<ExportStatement>(token, nullptr, std::move(specifiers));
    }

    throw Diagnostic::ParseErr("Đây không phải cú pháp 'export' hợp lệ...", peek());
}

StmtPtr Parser::expressionStatement(Token token) {
    ExprPtr expr = expression();
    consume(PUNCT_SEMICOLON, "Cần một dấu chấm phẩy ';' ở đây nhá bạn!");
    return std::make_unique<ExpressionStatement>(token, std::move(expr));
}

StmtPtr Parser::logStatement(Token token) {
    ExprPtr expr = expression();
    consume(PUNCT_SEMICOLON, "Cần một dấu chấm phẩy ';' ở đây nhá bạn!");
    return std::make_unique<LogStatement>(token, std::move(expr));
}

StmtPtr Parser::switchStatement(Token token) {
    ExprPtr valueToSwitch = expression();
    consume(PUNCT_LBRACE, "Cần một khối lệnh ngoặc nhọn '{' cho switch.");

    std::vector<std::unique_ptr<SwitchCase>> cases;

    while (!check(PUNCT_RBRACE) && !isAtEnd()) {
        ExprPtr caseValue = nullptr;
        Token caseToken = peek();
        if (match({KEYWORD_CASE})) {
            caseValue = expression();
        } else if (match({KEYWORD_DEFAULT})) {
        } else {
            throw Diagnostic::ParseErr("Mong đợi 'case' hoặc 'default' bên trong switch.", peek());
        }
        
        consume(PUNCT_COLON, "Thiếu dấu ':' sau giá trị của case/default.");

        std::vector<StmtPtr> statements;
        while (!check(PUNCT_RBRACE) && !check(KEYWORD_CASE) && !check(KEYWORD_DEFAULT) && !isAtEnd()) {
            statements.push_back(statement());
        }
        
        cases.push_back(std::make_unique<SwitchCase>(caseToken, std::move(caseValue), std::move(statements)));
    }

    consume(PUNCT_RBRACE, "Thiếu '}' để đóng khối lệnh switch.");

    return std::make_unique<SwitchStatement>(token, std::move(valueToSwitch), std::move(cases));
}

StmtPtr Parser::doWhileStatement(Token token) {
    StmtPtr body = statement();

    consume(KEYWORD_WHILE, "Thiếu từ khóa 'while' sau thân của vòng lặp 'do'.");

    ExprPtr condition = expression();

    consume(PUNCT_SEMICOLON, "Thiếu dấu ';' sau câu lệnh do-while.");

    return std::make_unique<DoWhileStatement>(token, std::move(body), std::move(condition));
}