#include "native_lib/standard_lib.hpp"
#include "runtime/interpreter.hpp"
#include "runtime/value.hpp"
#include "diagnostics/meow_exceptions.hpp"

#include <cmath>
#include <numeric>

Value print(Arguments args) {
    Str sep = " ";
    Str end = "\n";
    
    size_t argsCount = args.size();
    
    if (argsCount > 0 && std::holds_alternative<Object>(args.back())) {
        Object optionsObj = std::get<Object>(args.back());
        
        bool isOptionObject = false;

        auto sep_it = optionsObj->pairs.find(HashKey{Value("sep")});
        if (sep_it != optionsObj->pairs.end() && std::holds_alternative<String>(sep_it->second)) {
            sep = std::get<String>(sep_it->second)->str;
            isOptionObject = true;
        }

        auto end_it = optionsObj->pairs.find(HashKey{Value("end")});
        if (end_it != optionsObj->pairs.end() && std::holds_alternative<String>(end_it->second)) {
            end = std::get<String>(end_it->second)->str;
            isOptionObject = true;
        }
        if (isOptionObject) {
            argsCount--;
        }
    }

    for (size_t i = 0; i < argsCount; ++i) {
        std::cout << toString(args[i]);
        if (i < argsCount - 1) {
            std::cout << sep;
        }
    }

    std::cout << end;

    if (!end.empty() && end.back() == '\n') {
        std::cout << std::flush;
    }

    return Value(Null{});
}

Value typeOf(Arguments args) {
    return Value(visit([](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Null>) return "null";
        if constexpr (std::is_same_v<T, Int>) return "int";

        if constexpr (std::is_same_v<T, Real>) {
            Real r = arg;
            if (std::isinf(r)) return "real";
            if (std::isnan(r)) return "real";
            return "real";
        }
        if constexpr (std::is_same_v<T, Bool>) return "bool";

        if constexpr (std::is_same_v<T, String>) return "string";
        if constexpr (std::is_same_v<T, Array>) return "array";
        if constexpr (std::is_same_v<T, Object>) return "object";
        if constexpr (std::is_same_v<T, Function>) return "function";
        if constexpr (std::is_same_v<T, Class>) return "class";
        if constexpr (std::is_same_v<T, Instance>) return "instance";
        if constexpr (std::is_same_v<T, BoundMethod>) return "bound_method";
        return "unknown";
    }, args[0]));
}

Value toInt(Arguments args) {
    const auto& val = args[0];
    if (std::holds_alternative<Instance>(val)) {
        Instance instance = std::get<Instance>(val);
        Function method = instance->klass->findMethod("__int__");
        if (method) {
            return MeowScriptBoundMethod(instance, method).call(instance->engine, {});
        }
    }
    return visit(overloaded{
        [](Int i) { return Value(i); },

        [](Real r) -> Value {
            if (std::isinf(r)) {
                return (r > 0) ? Value(std::numeric_limits<Int>::max()) : Value(std::numeric_limits<Int>::min());
            }
            if (std::isnan(r)) {
                return Value(0);
            }
            return Value(static_cast<Int>(r));
        },
        [](Bool b) { return Value(Int(b ? 1 : 0)); },




        [](const String& s) {
            try {
                const std::string& raw = s->str;
                int base = 10;
                std::string num = raw;

                if (raw.size() > 2 && raw[0] == '0') {
                    if (raw[1] == 'b' || raw[1] == 'B') {
                        base = 2;
                        num = raw.substr(2);
                    } else if (raw[1] == 'x' || raw[1] == 'X') {
                        base = 16;
                        num = raw.substr(2);
                    } else if (raw[1] == 'o' || raw[1] == 'O') {
                        base = 8;
                        num = raw.substr(2);
                    }
                }

                return Value(static_cast<Int>(std::stoll(num, nullptr, base)));
            } catch (...) {
                throw FunctionException("Không thể chuyển chuỗi '" + s->str + "' thành số nguyên.");
            }
        },
        [](const auto&) -> Value {
            throw FunctionException("Không thể ép kiểu giá trị này thành số nguyên.");
        }
    }, val);
}

Value toReal(Arguments args) {
    const auto& val = args[0];
    if (std::holds_alternative<Instance>(val)) {
        Instance instance = std::get<Instance>(val);
        Function method = instance->klass->findMethod("__real__");
        if (method) {
            return MeowScriptBoundMethod(instance, method).call(instance->engine, {});
        }
    }
    return visit(overloaded{
        [](Int i) { return Value(static_cast<Real>(i)); },
        [](Real r) { return Value(r); },
        [](Bool b) { return Value(Real(b ? 1.0 : 0.0)); },









        [](const String& s) {
            try {
                if (s->str == "NaN") return Value(std::numeric_limits<Real>::quiet_NaN());
                if (s->str == "Infinity") return Value(std::numeric_limits<Real>::infinity());
                if (s->str == "-Infinity") return Value(-std::numeric_limits<Real>::infinity());
                return Value(std::stod(s->str));
            } catch (...) {
                throw FunctionException("Không thể chuyển chuỗi '" + s->str + "' thành số thực.");
            }
        },
        [](const auto&) -> Value {
            throw FunctionException("Không thể ép kiểu giá trị này thành số thực.");
        }
    }, val);
}

Value toBool(Arguments args) {
    const auto& val = args[0];
    if (std::holds_alternative<Instance>(val)) {
        Instance instance = std::get<Instance>(val);
        Function method = instance->klass->findMethod("__bool__");
        if (method) {
            return MeowScriptBoundMethod(instance, method).call(instance->engine, {});
        }
    }
    return visit(overloaded{
        [](Null) { return Value(false); },
        [](Int i) { return Value(i != 0); },

        [](Real r) { 
            return Value(r != 0.0 && !std::isnan(r));
        },
        [](Bool b) { return Value(b); },

        [](const String& s) { return Value(!s->str.empty()); },
        [](const Array& a) { return Value(!a->elements.empty()); },
        [](const Object& o) { return Value(!o->pairs.empty()); },
        [](const Function&) { return Value(true); },
        [](const Class&) { return Value(true); },
        [](const Instance&) { return Value(true); },
        [](const BoundMethod&) { return Value(true); }
    }, val);
}

Value toStr(Arguments args) {
    return Value(toString(args[0]));
}

Value toArray(Arguments args) {
    const auto& val = args[0];
    if (std::holds_alternative<Instance>(val)) {
        Instance instance = std::get<Instance>(val);
        Function method = instance->klass->findMethod("__array__");
        if (method) {
            return MeowScriptBoundMethod(instance, method).call(instance->engine, {});
        }
    }
    return visit(overloaded{





        [](const Array& a) {
            return Value(a);
        },
        [](const String& s) {
            auto arrData = std::make_shared<ArrayData>();
            for (char c : s->str) {
                arrData->elements.push_back(Value(std::make_shared<StringData>(1, c)));
            }
            return Value(arrData);
        },
        [](const Object& o) {
             auto arrData = std::make_shared<ArrayData>();
             for(const auto& pair : o->pairs){
                 arrData->elements.push_back(pair.second);
             }
             return Value(arrData);
        },
        [](const auto&) -> Value {
            throw FunctionException("Chỉ có thể ép kiểu Chuỗi hoặc Object thành Mảng.");
        }
    }, val);
}

Value toObject(Arguments args) {
    const auto& val = args[0];
    if (std::holds_alternative<Instance>(val)) {
        Instance instance = std::get<Instance>(val);
        Function method = instance->klass->findMethod("__object__");
        if (method) {
            return MeowScriptBoundMethod(instance, method).call(instance->engine, {});
        }
    }
    return visit(overloaded{
        [](const Object& o) -> Value {
            return Value(o);
        },
        [](const Array& a) -> Value {
            auto objData = std::make_shared<ObjectData>();
            for(const auto& item : a->elements){
                if(std::holds_alternative<Array>(item)){
                    const auto& pair = std::get<Array>(item)->elements;
                    if(pair.size() == 2 && isHashable(pair[0])){
                        objData->pairs.emplace(HashKey{pair[0]}, pair[1]);
                    } else {
                        throw FunctionException("Để ép kiểu thành Object, mảng con phải có dạng [key, value] và key phải hash được.");
                    }
                } else {
                    throw FunctionException("Để ép kiểu thành Object, mảng phải chứa các mảng con dạng [key, value].");
                }
            }
            return Value(objData);
        },
        [](const Instance& i) -> Value {
            auto newObj = std::make_shared<ObjectData>();
            newObj->pairs = i->fields->pairs;
            newObj->pairs.emplace(HashKey{Value("__class__")}, i->klass);
            return Value(newObj);
        },
        [](const Class& c) -> Value {
            auto objData = std::make_shared<ObjectData>();
            for(const auto& pair : c->static_fields){
                objData->pairs.emplace(HashKey{Value(pair.first)}, pair.second);
            }
            return Value(objData);
        },
        [](const auto&) -> Value {
            throw FunctionException("Chỉ có thể ép kiểu Mảng, Instance hoặc Class thành Object.");
        }
    }, val);
}

Value toInstance(Interpreter* engine, Arguments args) {
    const auto& val = args[0];
    return visit(overloaded{
        [](const Instance& i) -> Value {
            return Value(i);
        },
        [engine](const Object& o) -> Value {
            auto classIt = o->pairs.find(HashKey{Value("__class__")});
            if (classIt == o->pairs.end() || !std::holds_alternative<Class>(classIt->second)) {
                throw FunctionException("Object không có trường '__class__' hợp lệ để ép kiểu thành Instance.");
            }
            
            const Class& klass = std::get<Class>(classIt->second);
            
            Instance instance = std::make_shared<MeowScriptInstance>(klass, engine);
            instance->fields = o;
            
            return Value(instance);
        },
        [](const auto&) -> Value {
            throw FunctionException("Chỉ có thể ép kiểu Object thành Instance.");
        }
    }, val);
}

Value native_len(Arguments args) {
    const auto& val = args[0];
    return visit(overloaded{
        [](const String& s) { return Value((Int)s->str.length()); },
        [](const Array& a)  { return Value((Int)a->elements.size()); },
        [](const Object& o) { return Value((Int)o->pairs.size()); },
        [](const auto&) -> Value { 
            throw FunctionException("Hàm len() chỉ áp dụng cho chuỗi, mảng, hoặc object.");
        }
    }, val);
}

Value native_assert(Arguments args) {
    if (!isTruthy(args[0])) {
        std::string message = "Assertion failed.";
        if (args.size() > 1 && std::holds_alternative<String>(args[1])) {
            message = std::get<String>(args[1])->str;
        }
        throw FunctionException(message);
    }
    return Value(Null{});
}

Value native_string_ord(Arguments args) {
    const auto& str = std::get<String>(args[0])->str;
    if (str.length() != 1) {
        throw FunctionException("Hàm ord() chỉ chấp nhận chuỗi có đúng 1 ký tự.");
    }
    return Value((Int)static_cast<unsigned char>(str[0]));
}

Value native_string_chr(Arguments args) {
    Int code = std::get<Int>(args[0]);
    if (code < 0 || code > 255) {
        throw FunctionException("Mã ASCII của hàm chr() phải nằm trong khoảng [0, 255].");
    }
    return Value(std::string(1, static_cast<char>(code)));
}

Value native_range(Arguments args) {
    Int start = 0;
    Int stop = 0;
    Int step = 1;
    size_t argCount = args.size();

    if (argCount == 1) {
        stop = std::get<Int>(args[0]);
    } else if (argCount == 2) {
        start = std::get<Int>(args[0]);
        stop = std::get<Int>(args[1]);
    } else {
        start = std::get<Int>(args[0]);
        stop = std::get<Int>(args[1]);
        step = std::get<Int>(args[2]);
    }

    if (step == 0) {
        throw FunctionException("Tham số 'step' của hàm range() không thể bằng 0.");
    }

    auto resultArrayData = std::make_shared<ArrayData>();
    
    if (step > 0) {
        for (Int i = start; i < stop; i += step) {
            resultArrayData->elements.push_back(Value(i));
        }
    } else {
        for (Int i = start; i > stop; i += step) {
            resultArrayData->elements.push_back(Value(i));
        }
    }

    return Value(Array(resultArrayData));
}

CoreLib::CoreLib() {
    registerFn("print", print, Arity::atLeast(0));
    registerFn("len", native_len, 1);
    registerFn("assert", native_assert, Arity::range(1, 1));
    registerFn("typeof", typeOf, 1);

    registerFn("int", toInt, 1);
    registerFn("real", toReal, 1);
    registerFn("bool", toBool, 1);
    registerFn("str", toStr, 1);
    registerFn("array", toArray, 1);
    registerFn("object", toObject, 1);
    registerFn("instance", toInstance, 1);

    registerFn("range", native_range, Arity::range(1, 3));
    registerFn("ord", native_string_ord, 1);
    registerFn("chr", native_string_chr, 1);
}