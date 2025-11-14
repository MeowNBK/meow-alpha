#include "native_lib/standard_lib.hpp"
#include "diagnostics/meow_exceptions.hpp"



Value native_object_keys(Arguments args) {
    const auto& obj = std::get<Object>(args[0]);
    auto resultArr = std::make_shared<ArrayData>();
    for (const auto& pair : obj->pairs) {
        resultArr->elements.push_back(pair.first.value);
    }
    return Value(resultArr);
}

Value native_object_values(Arguments args) {
    const auto& obj = std::get<Object>(args[0]);
    auto resultArr = std::make_shared<ArrayData>();
    for (const auto& pair : obj->pairs) {
        resultArr->elements.push_back(pair.second);
    }
    return Value(resultArr);
}

Value native_object_entries(Arguments args) {
    const auto& obj = std::get<Object>(args[0]);
    auto resultArr = std::make_shared<ArrayData>();
    for (const auto& pair : obj->pairs) {
        auto entry = std::make_shared<ArrayData>();
        entry->elements.push_back(pair.first.value);
        entry->elements.push_back(pair.second);
        resultArr->elements.push_back(Value(entry));
    }
    return Value(resultArr);
}

Value native_object_has(Arguments args) {
    const auto& obj = std::get<Object>(args[0]);
    const auto& key = args[1];
    if (!isHashable(key)) {
        return Value(false);
    }
    return Value(obj->pairs.count(HashKey{key}) > 0);
}

Value native_object_merge(Arguments args) {
    auto resultObj = std::make_shared<ObjectData>();
    for (const auto& arg : args) {
        if (!std::holds_alternative<Object>(arg)) {
            throw FunctionException("Hàm merge() chỉ chấp nhận các tham số là object.");
        }
        const auto& objToMerge = std::get<Object>(arg);
        for (const auto& pair : objToMerge->pairs) {
            resultObj->pairs[pair.first] = pair.second;
        }
    }
    return Value(resultObj);
}


ObjectLib::ObjectLib() {
    registerFn("keys", native_object_keys, 1);
    registerFn("values", native_object_values, 1);
    registerFn("entries", native_object_entries, 1);
    registerFn("has", native_object_has, 2);
    registerFn("merge", native_object_merge, Arity::atLeast(1));
}