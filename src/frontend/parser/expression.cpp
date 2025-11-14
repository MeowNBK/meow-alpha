#include "parser/parser.hpp"

using enum TokenType;
using enum NodeType;

ExprPtr Parser::literal(Parser* parser) {
    const Token& prevToken = parser->previous();

    switch (prevToken.type) {
        case INTEGER:
            return std::make_unique<IntegerLiteral>(prevToken, std::stoll(prevToken.lexeme, nullptr, 0));
        case REAL:
            return std::make_unique<RealLiteral>(prevToken, std::stod(prevToken.lexeme));
        case STRING:
            return std::make_unique<StringLiteral>(prevToken);
        case BOOLEAN:
            return std::make_unique<BooleanLiteral>(prevToken, prevToken.lexeme == "true");
        case KEYWORD_NULL:
            return std::make_unique<NullLiteral>(prevToken);
        default:
            throw Diagnostic::ParseErr("Tôi chưa định nghĩa kiểu dữ liệu này, hay là bạn tự thêm nó à?", prevToken);
    }
}

ExprPtr Parser::arrayLiteral(Parser* parser) {
    const Token& token = parser->previous();
    std::vector<ExprPtr> elements;
    if (!parser->check(PUNCT_RBRACKET) && !parser->isAtEnd()) {
        do {
            if (parser->match({OP_ELLIPSIS})) {
                Token spreadToken = parser->previous();
                elements.push_back(std::make_unique<SpreadExpression>(spreadToken, parser->expression()));
            } else {
                elements.push_back(parser->expression());
            }
        } while (parser->match({PUNCT_COMMA}));
    }

    parser->consume(PUNCT_RBRACKET, "Cần một dấu ngoặc vuông phải ']'");

    return std::make_unique<ArrayLiteral>(token, std::move(elements));
}

ExprPtr Parser::functionLiteral(Parser *parser) {
    Token token = parser->previous();

    return parser->parseFunctionTail(std::move(token));
}

ExprPtr Parser::objectLiteral(Parser* parser) {
    const Token& token = parser->previous();

    std::vector<std::pair<ExprPtr, ExprPtr>> properties;

    while (!parser->check(PUNCT_RBRACE) && !parser->isAtEnd()) {
        ExprPtr key;
        if (parser->match({PUNCT_LBRACKET})) {
            key = parser->expression();
            parser->consume(PUNCT_RBRACKET, "Cần một dấu ngoặc vuông ']' sau key object");
        } else {
            Token keyToken = parser->peek();
            if (parser->match({IDENTIFIER, STRING})) {
                key = std::make_unique<StringLiteral>(keyToken);
            } else if (parser->match({INTEGER})) {
                key = std::make_unique<IntegerLiteral>(keyToken, std::stoll(keyToken.lexeme));
            } else if (parser->match({BOOLEAN})) {
                key = std::make_unique<BooleanLiteral>(keyToken, keyToken.lexeme == "true");
            } else {
                throw Diagnostic::ParseErr("Key của object không hợp lệ..", keyToken);
            }
        }

        parser->consume(PUNCT_COLON, "Cần một đấu hai chấm ':' sau key object");

        ExprPtr value = parser->expression();
        properties.emplace_back(std::move(key), std::move(value));

        if (!parser->match({PUNCT_COMMA})) {
            break;
        }
    }

    parser->consume(PUNCT_RBRACE, "Cần dấu ngoặc nhọn '}' trước khi kết thúc định nghĩa một object đấy!");

    return std::make_unique<ObjectLiteral>(token, std::move(properties));
}

ExprPtr Parser::templateLiteral(Parser* parser) {
    const Token& token = parser->previous();
    std::vector<ExprPtr> parts;

    while (!parser->check(PUNCT_BACKTICK) && !parser->isAtEnd()) {
        if (parser->match({STRING})) {
            parts.push_back(std::make_unique<StringLiteral>(parser->previous()));
        } else if (parser->match({PUNCT_PERCENT_LBRACE})) {
            parts.push_back(parser->expression());
            parser->consume(PUNCT_RBRACE, "Cần dấu ngoặc nhọn đòng '}' sau biểu thức này");
        }
    }
    parser->consume(PUNCT_BACKTICK, "Cần dấu backtick đóng '`' cho template");
    return std::make_unique<TemplateLiteral>(token, std::move(parts));
}

ExprPtr Parser::identifier(Parser* parser) {
    return std::make_unique<Identifier>(std::move(parser->previous()));
}

ExprPtr Parser::binary(Parser* parser, ExprPtr left) {
    const Token &op = parser->previous();

    auto right = parser->parsePrecedence(parser->rules[op.type].precedence);

    return make_unique<BinaryExpression>(op, std::move(left), std::move(right));
}

ExprPtr Parser::unary(Parser* parser) {
    const Token &op = parser->previous();

    ExprPtr operand = parser->parsePrecedence(Precedence::UNARY);

    return make_unique<UnaryExpression>(op, std::move(operand));
}

ExprPtr Parser::grouping(Parser* parser) {
    ExprPtr expr = parser->expression();
    parser->consume(PUNCT_RPAREN, "Một dấu ngoặc đơn ')' là điều không thể thiếu sau biểu thức. Thêm vào đi nhá!");
    return expr;
}

ExprPtr Parser::thisExpr(Parser* parser) {
    return std::make_unique<ThisExpression>(parser->previous());
}

ExprPtr Parser::superExpr(Parser* parser) {
    const Token& token = parser->previous();
    if (parser->check(PUNCT_LPAREN)) {
        return std::make_unique<SuperExpression>(token, true, nullptr);
    }
    parser->consume(PUNCT_DOT, "Sau super phải là một dấu chấm cho thuộc tính");

    const Token& propertyToken = parser->consume(IDENTIFIER, "Cần tên thuộc tính sau dấu chấm '.'");

    auto property = std::make_unique<Identifier>(propertyToken);

    return std::make_unique<SuperExpression>(token, false, std::move(property));
}

ExprPtr Parser::newExpr(Parser* parser) {
    const Token& token = parser->previous();
    const Token& identToken = parser->consume(IDENTIFIER, "Cần tên hàm để gọi sau 'new'");

    if (!parser->check(PUNCT_LPAREN)) {
        auto callExpr = std::make_unique<CallExpression>(identToken, std::make_unique<Identifier>(identToken), std::vector<ExprPtr>{});
        return std::make_unique<NewExpression>(token, std::move(callExpr));
    } else {
        parser->consume(PUNCT_LPAREN, "Cần '(' sau tên class");
        auto callExpr = Parser::call(parser, std::make_unique<Identifier>(identToken));
        return std::make_unique<NewExpression>(token, std::move(callExpr));
    }

    throw Diagnostic::ParseErr("Không dùng cái này sau 'new' được", token);
}

ExprPtr Parser::spreadExpr(Parser* parser) {
    const Token& token = parser->previous();

    ExprPtr expr = parser->expression();

    return std::make_unique<SpreadExpression>(token, std::move(expr));
}

ExprPtr Parser::prefixUpdate(Parser* parser) {
    const Token &token = parser->previous();
    ExprPtr expr = parser->expression();
    return std::make_unique<PrefixUpdateExpression>(token, std::move(expr));
}

ExprPtr Parser::assignment(Parser* parser, ExprPtr left) {
    const Token& op = parser->previous();
    ExprPtr value = parser->parsePrecedence((Precedence)(static_cast<int>(Precedence::ASSIGN) - 1));

    if (left->type == EXPR_IDENTIFIER || left->type == EXPR_INDEX || left->type == EXPR_PROPERTY_ACCESS) {
        return std::make_unique<AssignmentExpression>(op, std::move(left), std::move(value));
    }

    throw Diagnostic::ParseErr("Đối tượng được gán không hợp lệ!", op);
}

ExprPtr Parser::compoundAssignment(Parser* parser, ExprPtr left) {
    const Token& op = parser->previous();

    if (left->type != EXPR_IDENTIFIER && left->type != EXPR_INDEX && left->type != EXPR_PROPERTY_ACCESS) {
        throw Diagnostic::ParseErr("Đối tượng được gán không hợp lệ đâu! Thử cái khác đi bạn!", op);
    }

    ExprPtr rightValue = parser->parsePrecedence((Precedence)(static_cast<int>(Precedence::ASSIGN) - 1));

    TokenType opType;
    switch (op.type) {
        case OP_PLUS_ASSIGN:   opType = OP_PLUS; break;
        case OP_MINUS_ASSIGN:  opType = OP_MINUS; break;
        case OP_MULTIPLY_ASSIGN: opType = OP_MULTIPLY; break;
        case OP_DIVIDE_ASSIGN: opType = OP_DIVIDE; break;
        case OP_MODULO_ASSIGN: opType = OP_MODULO; break;
        case OP_EXPONENT_ASSIGN: opType = OP_EXPONENT; break;

        case OP_AND_ASSIGN: opType = OP_BIT_AND; break;
        case OP_OR_ASSIGN:  opType = OP_BIT_OR; break;
        case OP_XOR_ASSIGN: opType = OP_BIT_XOR; break;
        case OP_LSHIFT_ASSIGN:  opType = OP_LSHIFT; break;
        case OP_RSHIFT_ASSIGN: opType = OP_RSHIFT; break;
        default:
            throw Diagnostic::ParseErr("Ayya, toán tử gán này tôi chưa hỗ trợ. Hay là bạn bảo tôi bổ sung nhá!", op);
    }
    ExprPtr leftCloned = ExprPtr(dynamic_cast<Expression*>(left->clone().release()));

    Token opToken = Token(opType, op.lexeme, op.filename, op.line, op.col, op.srcFile);
    ExprPtr expr = std::make_unique<BinaryExpression>(opToken, std::move(leftCloned), std::move(rightValue));

    Token assignToken = Token(OP_ASSIGN, "=", op.filename, op.line, op.col, op.srcFile);
    return std::make_unique<AssignmentExpression>(assignToken, std::move(left), std::move(expr));
}

ExprPtr Parser::call(Parser* parser, ExprPtr left) {
    std::vector<ExprPtr> args;

    if (!parser->check(PUNCT_RPAREN)) {
        do {
            if (parser->match({OP_ELLIPSIS})) {
                Token spreadToken = parser->previous();
                args.push_back(std::make_unique<SpreadExpression>(spreadToken, parser->expression()));
            } else {
                args.push_back(parser->expression());
            }
        } while (parser->match({PUNCT_COMMA}));
    }

    const Token& closingParen = parser->consume(PUNCT_RPAREN, "Yo, gọi hàm mà quên dấu ngoặc đơn ')' à?");

    return std::make_unique<CallExpression>(closingParen, std::move(left), std::move(args));
}

ExprPtr Parser::index(Parser* parser, ExprPtr left) {
    ExprPtr expr = parser->expression();
    const Token& closingBracket = parser->consume(PUNCT_RBRACKET, "Thiếu luôn dấu ngoặc vuông ']' khi kết thúc truy cập. Hay thật!");

    return std::make_unique<IndexExpression>(closingBracket, std::move(left), std::move(expr));
}

ExprPtr Parser::access(Parser* parser, ExprPtr left) {
    const Token& token = parser->previous();
    const Token& propertyToken = parser->consume(IDENTIFIER, "Cần tên thuộc tính sau dấu chấm '.'");

    auto property = std::make_unique<Identifier>(propertyToken);

    return std::make_unique<PropertyAccess>(token, std::move(left), std::move(property));
}

ExprPtr Parser::ternary(Parser* parser, ExprPtr left) {
    const Token& token = parser->previous();

    ExprPtr thenBranch = parser->expression();
    parser->consume(PUNCT_COLON, "Sau nhánh 'then' thì cần dấu hai chấm ':' mà bạn!");

    ExprPtr elseBranch = parser->parsePrecedence((Precedence)(static_cast<int>(Precedence::TERNARY) - 1));

    return std::make_unique<TernaryExpression>(token, std::move(left), std::move(thenBranch), std::move(elseBranch));
}

ExprPtr Parser::postfixUpdate(Parser* parser, ExprPtr left) {
    return std::make_unique<PostfixUpdateExpression>(parser->previous(), std::move(left));
}

ExprPtr Parser::parseFunctionTail(Token token) {
    consume(PUNCT_LPAREN, "Này này, bạn quên dấu ngoặc đơn '(' để bắt đầu cho những tham số đấy nhá!");

    std::vector<std::unique_ptr<Identifier>> params;
    IdenPtr restParam = nullptr;

    if (!check(PUNCT_RPAREN) && !isAtEnd()) {
        do {
            if (match({OP_ELLIPSIS})) {
                restParam = std::make_unique<Identifier>(consume(IDENTIFIER, "Sau '...' phải là một tên biến!"));
                break;
            }
            params.push_back(std::make_unique<Identifier>(consume(IDENTIFIER, "Đây phải là tên tham số mà bạn!")));
        } while (match({PUNCT_COMMA}));
    }

    consume(PUNCT_RPAREN, "Thiếu luôn một dấu ngoặc đơn ')'. Tạch rồi");

    StmtPtr body = declaration();

    return std::make_unique<FunctionLiteral>(std::move(token), std::move(params), std::move(body), std::move(restParam));
}