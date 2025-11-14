#include "native_lib/standard_lib.hpp"
#include "diagnostics/meow_exceptions.hpp"
#include <random>
#include <algorithm>

namespace {



    static std::mt19937 random_engine(std::random_device{}());
}



Value native_random_random([[maybe_unused]] Arguments args) {
    std::uniform_real_distribution<double> distrib(0.0, 1.0);
    return Value(distrib(random_engine));
}

Value native_random_randint(Arguments args) {
    Int min = std::get<Int>(args[0]);
    Int max = std::get<Int>(args[1]);
    if (min > max) {
        throw FunctionException("Trong hàm randint, 'min' không được lớn hơn 'max'.");
    }
    std::uniform_int_distribution<Int> distrib(min, max);
    return Value(distrib(random_engine));
}

Value native_random_choice(Arguments args) {
    const auto& sequence = args[0];

    return visit(overloaded {
        [](const String& s) -> Value {
            if (s->str.empty()) {
                throw FunctionException("Không thể chọn từ một chuỗi rỗng.");
            }
            std::uniform_int_distribution<size_t> distrib(0, s->str.length() - 1);
            size_t index = distrib(random_engine);
            return Value(std::string(1, s->str[index]));
        },
        [](const Array& a) -> Value {
            if (a->elements.empty()) {
                throw FunctionException("Không thể chọn từ một mảng rỗng.");
            }
            std::uniform_int_distribution<size_t> distrib(0, a->elements.size() - 1);
            size_t index = distrib(random_engine);
            return a->elements[index];
        },
        [](const auto&) -> Value {
            throw FunctionException("Hàm choice() chỉ áp dụng cho chuỗi hoặc mảng.");
        }
    }, sequence);
}

Value native_random_shuffle(Arguments args) {
    auto& arr = std::get<Array>(args[0]);
    std::shuffle(arr->elements.begin(), arr->elements.end(), random_engine);
    return Value(Null{});
}



RandomLib::RandomLib() {
    registerFn("random", native_random_random, 0);
    registerFn("randint", native_random_randint, 2);
    registerFn("choice", native_random_choice, 1);
    registerFn("shuffle", native_random_shuffle, 1);
}