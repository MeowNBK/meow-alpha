#include "visitor/tree_walker.hpp"
#include "callable/function_callable.hpp"
#include "common/ast.hpp"
#include "diagnostics/diagnostic.hpp"
#include <functional>
#include <sstream>

Value TreeWalker::visit(IntegerLiteral* node) {
    return Value(node->value);
}
Value TreeWalker::visit(RealLiteral* node) {
    return Value(node->value);
}
Value TreeWalker::visit(StringLiteral* node) {
    return Value(std::make_shared<StringData>(node->value));
}
Value TreeWalker::visit(BooleanLiteral* node) {
    return Value(node->value);
}
Value TreeWalker::visit(NullLiteral* node) {
    return Value(Null{});
}

Value TreeWalker::visit(ArrayLiteral* node) {
    auto data = std::make_shared<ArrayData>();

    for (const auto& element : node->elements) {
        if (auto spread = dynamic_cast<SpreadExpression*>(element.get())) {
            Value collection = evaluate(spread->expression.get());

            if (auto iterable = toIterable(collection)) {
                auto iterator = iterable->makeIterator();
                while (iterator->hasNext()) {
                    data->elements.push_back(iterator->next());
                }
            } else {
                throwRuntimeErr(spread->token, "Toán tử '...' chỉ có thể dùng với các kiểu có thể duyệt qua (iterable).");
            }
        } else {
            data->elements.push_back(evaluate(element.get()));
        }
    }
    return Value(Array(data));
}

Value TreeWalker::visit(ObjectLiteral* node) {
    auto obj = std::make_shared<ObjectData>();

    for (const auto& pair : node->properties) {
        Value key = evaluate(pair.first.get());
        Value value = evaluate(pair.second.get());

        try {
            obj->pairs[HashKey{key}] = value;
        } catch (std::runtime_error& e) {
            this->throwRuntimeErr(node->token, e.what());
        }
    }

    return Value(Object(obj));
}

Value TreeWalker::visit(FunctionLiteral* node) {
    Function function = std::make_shared<MeowScriptFunction>(node, this->env);
    
    return Value(function);
}

Value TreeWalker::visit(TemplateLiteral* node) {
    std::ostringstream os;

    for (const auto& part : node->parts) {
        os << toString(evaluate(part.get()));
    }

    return Value(os.str());
}