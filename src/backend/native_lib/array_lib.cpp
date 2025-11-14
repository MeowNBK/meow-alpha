#include "native_lib/standard_lib.hpp"
#include "runtime/interpreter.hpp"
#include "diagnostics/meow_exceptions.hpp"
#include <sstream>



Value native_array_push(Arguments args) {
    auto& arr = std::get<Array>(args[0]);
    for (size_t i = 1; i < args.size(); ++i) {
        arr->elements.push_back(args[i]);
    }

    return Value((Int)arr->elements.size());
}

Value native_array_pop(Arguments args) {
    auto& arr = std::get<Array>(args[0]);
    if (arr->elements.empty()) {
        return Value(Null{});
    }
    Value lastElement = arr->elements.back();
    arr->elements.pop_back();
    return lastElement;
}

Value native_array_slice(Arguments args) {
    auto& arr = std::get<Array>(args[0]);
    Int start = 0;
    Int end = arr->elements.size();

    if (args.size() > 1) {
        start = std::get<Int>(args[1]);
    }
    if (args.size() > 2) {
        end = std::get<Int>(args[2]);
    }
    if (start < 0) start += arr->elements.size();
    if (end < 0) end += arr->elements.size();

    start = std::max((Int)0, start);
    end = std::min((Int)arr->elements.size(), end);

    auto newArrData = std::make_shared<ArrayData>();
    if (start < end) {
        for (Int i = start; i < end; ++i) {
            newArrData->elements.push_back(arr->elements[i]);
        }
    }
    
    return Value(newArrData);
}

Value native_array_map(Interpreter* engine, Arguments args) {
    const auto& arr = std::get<Array>(args[0]);
    const auto& callback = std::get<Function>(args[1]);
    
    auto newArrData = std::make_shared<ArrayData>();
    newArrData->elements.reserve(arr->elements.size());

    for (const auto& element : arr->elements) {

        Value result = engine->call(callback, {element});
        newArrData->elements.push_back(result);
    }

    return Value(newArrData);
}

Value native_array_filter(Interpreter* engine, Arguments args) {
    const auto& arr = std::get<Array>(args[0]);
    const auto& callback = std::get<Function>(args[1]);

    auto newArrData = std::make_shared<ArrayData>();

    for (const auto& element : arr->elements) {

        Value result = engine->call(callback, {element});
        if (isTruthy(result)) {

            newArrData->elements.push_back(element);
        }
    }

    return Value(newArrData);
}

Value native_array_reduce(Interpreter* engine, Arguments args) {
    const auto& arr = std::get<Array>(args[0]);
    const auto& callback = std::get<Function>(args[1]);
    Value accumulator = args[2];

    for (const auto& element : arr->elements) {

        accumulator = engine->call(callback, {accumulator, element});
    }

    return accumulator;
}

Value native_array_forEach(Interpreter* engine, Arguments args) {
    const auto& arr = std::get<Array>(args[0]);
    const auto& callback = std::get<Function>(args[1]);

    for (size_t i = 0; i < arr->elements.size(); ++i) {

        engine->call(callback, {arr->elements[i], Value((Int)i)});
    }

    return Value(Null{});
}

Value native_array_find(Interpreter* engine, Arguments args) {
    const auto& arr = std::get<Array>(args[0]);
    const auto& callback = std::get<Function>(args[1]);

    for (size_t i = 0; i < arr->elements.size(); ++i) {
        Value result = engine->call(callback, {arr->elements[i], Value((Int)i)});
        if (isTruthy(result)) {
            return arr->elements[i];
        }
    }

    return Value(Null{});
}

Value native_array_findIndex(Interpreter* engine, Arguments args) {
    const auto& arr = std::get<Array>(args[0]);
    const auto& callback = std::get<Function>(args[1]);

    for (size_t i = 0; i < arr->elements.size(); ++i) {
        Value result = engine->call(callback, {arr->elements[i], Value((Int)i)});
        if (isTruthy(result)) {
            return Value((Int)i);
        }
    }

    return Value((Int)-1);
}

Value native_array_reverse(Arguments args) {
    auto& arr = std::get<Array>(args[0]);
    std::reverse(arr->elements.begin(), arr->elements.end());
    return args[0];
}

Value native_array_sort(Interpreter* engine, Arguments args) {
    auto& arr_val = std::get<Array>(args[0]);
    auto& elements = arr_val->elements;

    std::sort(elements.begin(), elements.end(),
        [&](const Value& a, const Value& b) -> bool {
            

            if (args.size() > 1) {
                const auto& callback = std::get<Function>(args[1]);
                Value result = engine->call(callback, {a, b});

                if (std::holds_alternative<Int>(result)) {
                    return std::get<Int>(result) < 0;
                }
                if (std::holds_alternative<Real>(result)) {
                    return std::get<Real>(result) < 0.0;
                }

                return false; 
            }

            return visit(overloaded {

                [](const auto& l, const auto& r) -> bool 
                    requires (std::is_arithmetic_v<decltype(l)> && std::is_arithmetic_v<decltype(r)>) 
                {
                    return l < r;
                },

                [](const String& l, const String& r) {
                    return l->str < r->str;
                },

                [](const auto&, const auto&) -> bool {
                    throw FunctionException("Không thể so sánh các giá trị có kiểu khác nhau trong sắp xếp mặc định.");
                }
            }, static_cast<const BaseValue&>(a), static_cast<const BaseValue&>(b));
        });

    return args[0];
}


Value native_array_reserve(Arguments args) {
    auto& arr = std::get<Array>(args[0]);
    Int capacity = std::get<Int>(args[1]);
    if (capacity < 0) {
        throw FunctionException("Dung lượng reserve không được âm.");
    }
    arr->elements.reserve((size_t)capacity);
    return Value(Null{});
}

Value native_array_resize(Arguments args) {
    auto& arr = std::get<Array>(args[0]);
    Int newSize = std::get<Int>(args[1]);
    if (newSize < 0) {
        throw FunctionException("Kích thước mới không được âm.");
    }

    if (args.size() > 2) {
        arr->elements.resize((size_t)newSize, args[2]);
    } else {
        arr->elements.resize((size_t)newSize, Value(Null{}));
    }

    return Value(Null{});
}

Value native_array_size(Arguments args) {
    auto& arr = std::get<Array>(args[0]);
    return Value((Int)arr->elements.size());
}


ArrayLib::ArrayLib() {
    registerFn("push", native_array_push, Arity::atLeast(2));
    registerFn("pop", native_array_pop, 1);
    registerFn("slice", native_array_slice, Arity::range(1, 2));

    registerFn("map", native_array_map, 2);
    registerFn("filter", native_array_filter, 2);
    registerFn("reduce", native_array_reduce, 3);

    registerFn("forEach", native_array_forEach, 2);
    registerFn("find", native_array_find, 2);
    registerFn("findIndex", native_array_findIndex, 2);
    registerFn("reverse", native_array_reverse, 1);
    registerFn("sort", native_array_sort, Arity::range(1, 1));

    registerFn("reserve", native_array_reserve, 2);
    registerFn("resize", native_array_resize, Arity::range(2, 3));

    registerFn("size", native_array_size, 1);
}