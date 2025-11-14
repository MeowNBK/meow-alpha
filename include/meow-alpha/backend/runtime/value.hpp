#pragma once

#include "runtime/overload.hpp"
#include "runtime/function_arity.hpp"
#include "runtime/oop.hpp"
#include "interface/indexable.hpp"
#include "interface/iterable.hpp"
#include "interface/stringifiable.hpp"

#include "native_lib/native_lib.hpp"

#include <variant>
#include <cstdint>
#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

class NativeLibrary;

extern const NativeLibrary* arrayLib;
extern const NativeLibrary* objectLib;
extern const NativeLibrary* stringLib;

void initNativeMethods(const NativeLibrary& arr, const NativeLibrary& obj, const NativeLibrary& str);

class Callable;
class Interpreter;
class ArrayIterator;
class ObjectIterator;
struct ArrayData;
struct ObjectData;
struct StringData;

using Null = std::monostate;
using Int = int64_t;
using Real = double;
using Bool = bool;
using Str = std::string;
using Array = std::shared_ptr<ArrayData>;
using Object = std::shared_ptr<ObjectData>;
using String = std::shared_ptr<StringData>;
using Function = std::shared_ptr<Callable>;

using BaseValue = std::variant<Null, Int, Real, Bool, String, Array, Object, Function, Class, Instance, BoundMethod>;

class Value : public BaseValue {
public:
    Value() = default;
    Value(const char* s) : BaseValue(std::make_shared<StringData>(s)) {}
    Value(const std::string& s) : BaseValue(std::make_shared<StringData>(s)) {}
    template<typename T>
        requires (!std::is_same_v<std::decay_t<T>, Value> && !std::is_convertible_v<T, std::string>)
    Value(T&& t) : BaseValue(std::forward<T>(t)) {}
};

std::ostream& operator<<(std::ostream& os, const Value& v);
Str toString(const Value& v);

struct ArrayData: public Indexable, public Iterable, public Stringifiable, public std::enable_shared_from_this<ArrayData> {
    std::vector<Value> elements;

    Value get(const Value& key) override;
    void set(const Value& key, const Value& value) override;
    std::unique_ptr<Iterator> makeIterator() override;
    std::string toString() const override;

    ~ArrayData() override = default;
};

class ArrayIterator : public Iterator {
private:
    ArrayData* arrayData;
    size_t current = 0;
public:
    ArrayIterator(ArrayData* data);
    bool hasNext() const override;
    Value next() override;
};

inline bool isHashable(const Value& value) {
    return std::holds_alternative<Int>(value) || std::holds_alternative<Bool>(value) || std::holds_alternative<String>(value);
}

bool operator==(const Value& lhs, const Value& rhs);

inline bool operator!=(const Value& lhs, const Value& rhs) {
    return !(lhs == rhs);
}

struct HashKey {
    Value value;
    bool operator==(const HashKey& other) const;
};

struct Hasher {
    size_t operator()(const Value& v) const;
};

template<>
struct std::hash<HashKey> {
    size_t operator()(const HashKey& key) const;
};

struct ObjectData: public Indexable, public Iterable, public Stringifiable, public std::enable_shared_from_this<ObjectData> {
    std::unordered_map<HashKey, Value> pairs;

    Value get(const Value& key) override;
    void set(const Value& key, const Value& value) override;
    std::unique_ptr<Iterator> makeIterator() override;
    std::string toString() const override;
};

class ObjectIterator : public Iterator {
private:
    std::unordered_map<HashKey, Value>::iterator current;
    std::unordered_map<HashKey, Value>::iterator end;

public:
    ObjectIterator(ObjectData* data);
    bool hasNext() const override;
    Value next() override;
};

struct StringData : public Indexable, public Iterable, public Stringifiable, public std::enable_shared_from_this<StringData> {
    std::string str;

    StringData(std::string val): str(std::move(val)) {}
    StringData(int count, char ch) : str(count, ch) {}

    Value get(const Value& key) override;
    void set(const Value& key, const Value& value) override;
    std::unique_ptr<Iterator> makeIterator() override;
    std::string toString() const override;
};

class StringIterator : public Iterator {
private:
    StringData* strData;
    size_t current = 0;
public:
    StringIterator(StringData* data);
    bool hasNext() const override;
    Value next() override;
};

inline bool isTruthy(const Value& value) {
    return !std::holds_alternative<Null>(value) && (!std::holds_alternative<Bool>(value) || std::get<Bool>(value));
}

template<class Visitor>
auto visit(Visitor&& vis, const Value& val) {
    return std::visit(vis, static_cast<const BaseValue&>(val));
}

template<class Visitor>
auto visit(Visitor&& vis, const Value& val1, const Value& val2) {
    return std::visit(vis,
        static_cast<const BaseValue&>(val1),
        static_cast<const BaseValue&>(val2)
    );
}