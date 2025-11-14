#include "native_lib/standard_lib.hpp"
#include "runtime/value.hpp"
#include "diagnostics/meow_exceptions.hpp"
#include <functional>
#include <cmath>
#include <numbers>

template<typename Func>
NativeFnSimple unaryFunc(Func func) {
    return [func](const std::vector<Value>& args) -> Value {
        const Value& input = args[0];
        double inputAsDouble = 0.0;

        if (std::holds_alternative<Real>(input)) {
            inputAsDouble = std::get<Real>(input);
        } else if (std::holds_alternative<Int>(input)) {
            inputAsDouble = static_cast<double>(std::get<Int>(input));
        } else {
            throw FunctionException("Hàm này chỉ nhận tham số là số (Number).");
        }
        
        return func(inputAsDouble);

    };
}

template<typename Func>
NativeFnSimple binaryFunc(Func func) {
    return [func](const std::vector<Value>& args) -> Value {
        const Value& left = args[0];
        const Value& right = args[1];

        auto toReal = [](const Value& v) -> double {
            if (std::holds_alternative<Real>(v)) return std::get<Real>(v);
            if (std::holds_alternative<Int>(v)) return static_cast<double>(std::get<Int>(v));
            return std::numeric_limits<double>::quiet_NaN();
        };

        double leftReal = toReal(left);
        double rightReal = toReal(right);

        if (std::isnan(leftReal) || std::isnan(rightReal)) {
            throw FunctionException("Hàm này chỉ nhận 2 tham số là số (Number).");
        }
        
        return func(leftReal, rightReal);
    };
}


enum CompareResult { LESS, GREATER, EQUAL };
CompareResult compareValues(const Value& a, const Value& b) {

    double num_a = std::holds_alternative<Int>(a) ? std::get<Int>(a) : std::get<Real>(a);
    double num_b = std::holds_alternative<Int>(b) ? std::get<Int>(b) : std::get<Real>(b);
    if (num_a < num_b) return LESS;
    if (num_a > num_b) return GREATER;
    return EQUAL;
}

Value native_math_min(Arguments args) {
    Value minVal = args[0];
    for (size_t i = 1; i < args.size(); ++i) {
        if (compareValues(args[i], minVal) == LESS) {
            minVal = args[i];
        }
    }
    return minVal;
}

Value native_math_max(Arguments args) {
    Value maxVal = args[0];
    for (size_t i = 1; i < args.size(); ++i) {
        if (compareValues(args[i], maxVal) == GREATER) {
            maxVal = args[i];
        }
    }
    return maxVal;
}

using cast = double(*)(double,double);

MathLib::MathLib() {

    registerValue("PI", Value(std::numbers::pi));
    registerValue("E", Value(std::numbers::e));

    registerFn("sin",   unaryFunc(static_cast<double(*)(double)>(&std::sin)),   1);
    registerFn("cos",   unaryFunc(static_cast<double(*)(double)>(&std::cos)),   1);
    registerFn("tan",   unaryFunc(static_cast<double(*)(double)>(&std::tan)),   1);
    registerFn("asin",  unaryFunc(static_cast<double(*)(double)>(&std::asin)),  1);
    registerFn("acos",  unaryFunc(static_cast<double(*)(double)>(&std::acos)),  1);
    registerFn("atan",  unaryFunc(static_cast<double(*)(double)>(&std::atan)),  1);

    registerFn("sqrt",  unaryFunc(static_cast<double(*)(double)>(&std::sqrt)),  1);
    registerFn("cbrt",  unaryFunc(static_cast<double(*)(double)>(&std::cbrt)),  1);
    registerFn("exp",   unaryFunc(static_cast<double(*)(double)>(&std::exp)),   1);
    registerFn("log",   unaryFunc(static_cast<double(*)(double)>(&std::log)),   1);
    registerFn("log10", unaryFunc(static_cast<double(*)(double)>(&std::log10)), 1);
    registerFn("log2",  unaryFunc(static_cast<double(*)(double)>(&std::log2)),  1);
    

    registerFn("abs",   unaryFunc(static_cast<double(*)(double)>(&std::fabs)),  1);
    registerFn("floor", unaryFunc(static_cast<double(*)(double)>(&std::floor)), 1);
    registerFn("ceil",  unaryFunc(static_cast<double(*)(double)>(&std::ceil)),  1);
    registerFn("round", unaryFunc(static_cast<double(*)(double)>(&std::round)), 1);
    registerFn("trunc", unaryFunc(static_cast<double(*)(double)>(&std::trunc)), 1);

    registerFn("pow",   binaryFunc(static_cast<double(*)(double, double)>(&std::pow)),   2);
    registerFn("atan2", binaryFunc(static_cast<double(*)(double, double)>(&std::atan2)), 2);
    registerFn("hypot", binaryFunc(static_cast<double(*)(double, double)>(&std::hypot)), 2);
    
    registerFn("min", native_math_min, Arity::atLeast(1));
    registerFn("max", native_math_max, Arity::atLeast(1));
}