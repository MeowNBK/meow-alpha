#include "runtime/value.hpp"
#include "native_lib/standard_lib.hpp"
#include "runtime/native_fn.hpp"
#include <stdexcept>
#include <sstream>

#include <numeric>
#include <cmath>

const NativeLibrary* arrayLib = nullptr;
const NativeLibrary* objectLib = nullptr;
const NativeLibrary* stringLib = nullptr;

void initNativeMethods(const NativeLibrary& arr, const NativeLibrary& obj, const NativeLibrary& str) {
    arrayLib = &arr;
    objectLib = &obj;
    stringLib = &str;
}

Value ArrayData::get(const Value& key) {
    if (std::holds_alternative<String>(key)) {
        const auto& propName = std::get<String>(key)->str;
        
        if (propName == "length") {
            return Value((Int)elements.size());
        }

        auto it = arrayLib->contents.find(propName);
        if (it != arrayLib->contents.end()) {
            const auto& unboundFunction = std::get<Function>(it->second);
            NativeFnAdvanced boundedFunction = [self = this, unboundFunction](Interpreter* engine, Arguments args) -> Value {
                std::vector<Value> finalArgs;
                finalArgs.push_back(Value(self->shared_from_this()));
                finalArgs.insert(finalArgs.end(), args.begin(), args.end());
                return unboundFunction->call(engine, finalArgs);
            };
            Arity newArity = unboundFunction->arity();
            newArity.required = std::max(0, newArity.required - 1);
            auto data = std::make_shared<NativeFunction>(propName, boundedFunction, newArity);
            return Value(Function(std::make_shared<NativeCallable>(data)));
        }
    }

    if (std::holds_alternative<Int>(key)) {
        Int idx = std::get<Int>(key);
        if (static_cast<size_t>(idx) >= elements.size() || idx < 0) {
            throw std::runtime_error("Chỉ số không nằm trong phạm vi truy cập phần tử!");
        }
        return Value(elements[idx]);
    }
    
    throw std::runtime_error("Không thể truy cập mảng bằng key '" + ::toString(key) + "'.");
}

void ArrayData::set(const Value& key, const Value& value) {
    if (!std::holds_alternative<Int>(key)) {
        throw std::runtime_error("Chỉ số 'key' khác số nguyên?");
    }

    Int idx = std::get<Int>(key);

    if (static_cast<size_t>(idx) >= elements.size() || idx < 0) {
        throw std::runtime_error("Chỉ số không nằm trong phạm vi truy cập phần tử!");
    }

    elements[idx] = value;
}

std::unique_ptr<Iterator> ArrayData::makeIterator() {
    return std::make_unique<ArrayIterator>(this);
}

std::string ArrayData::toString() const {
    std::ostringstream os;
    os << "[";
    if (elements.empty()) {
        os << "]";
        return os.str();
    }
    for (size_t i = 0; i < elements.size(); ++i) {
        os << elements[i];
        if (i < elements.size() - 1) os << ", ";
    }
    os << "]";

    return os.str();
}

ArrayIterator::ArrayIterator(ArrayData* data) : arrayData(data) {}

bool ArrayIterator::hasNext() const { 
    return current < arrayData->elements.size(); 
}

Value ArrayIterator::next() { 
    return arrayData->elements[current++]; 
}

NativeFunction::NativeFunction(const std::string& n, NativeFnSimple fn, Arity a): name(n), function(fn), arity(a) {}
NativeFunction::NativeFunction(const std::string& n, NativeFnAdvanced fn, Arity a): name(n), function(fn), arity(a) {}

bool HashKey::operator==(const HashKey& other) const {
    return this->value == other.value;
}

bool operator==(const Value& lhs, const Value& rhs) {
    if (lhs.index() != rhs.index()) {
        if ((std::holds_alternative<Int>(lhs) && std::holds_alternative<Real>(rhs)) ||
            (std::holds_alternative<Real>(lhs) && std::holds_alternative<Int>(rhs))) {
        } else {
            return false;
        }
    }

    return std::visit(overloaded {
        [](const String& l, const String& r) { return l->str == r->str; },
        [](const Array&, const Array&)   { return false; },
        [](const Object&, const Object&) { return false; },

        [](const Int& l, const Int& r)       { return l == r; },
        [](const Real& l, const Real& r)     { return l == r; },
        [](const Bool& l, const Bool& r)     { return l == r; },
        [](const Null&, const Null&) { return true; },

        [](const auto&, const auto&)         { return false; }
    }, static_cast<const BaseValue&>(lhs), static_cast<const BaseValue&>(rhs));
}

size_t Hasher::operator()(const Value& v) const {
    return std::visit(overloaded {
        [](const Int& i)      { return std::hash<Int>{}(i); },
        [](const Bool& b)     { return std::hash<Bool>{}(b); },
        [](const String& s)   { return std::hash<Str>{}(s->str); },
        [](const auto&) -> size_t {
            throw std::runtime_error("Hm... bạn đang cố dùng một key không 'hash' được?");
        }
    }, static_cast<const BaseValue&>(v));
}

size_t std::hash<HashKey>::operator()(const HashKey& key) const {
    if (!isHashable(key.value)) {
        throw std::runtime_error("Cái này không dùng làm key được! " + toString(key.value));
    }
    return Hasher{}(key.value);
}

Value ObjectData::get(const Value& key) {
    if (!isHashable(key)) {
        throw std::runtime_error("Không thể dùng key với kiểu dữ liệu này");
    }

    auto it = pairs.find(HashKey{key});
    if (it != pairs.end()) {
        return it->second;
    }

    if (std::holds_alternative<String>(key)) {
        const auto& propName = std::get<String>(key)->str;

        auto it = objectLib->contents.find(propName);
        if (it != objectLib->contents.end()) {
            const auto& unboundFunction = std::get<Function>(it->second);
            NativeFnAdvanced boundedFunction = [self = this, unboundFunction](Interpreter* engine, Arguments args) -> Value {
                std::vector<Value> finalArgs;
                finalArgs.push_back(Value(self->shared_from_this()));
                finalArgs.insert(finalArgs.end(), args.begin(), args.end());
                return unboundFunction->call(engine, finalArgs);
            };
            Arity newArity = unboundFunction->arity();
            newArity.required = std::max(0, newArity.required - 1);
            auto data = std::make_shared<NativeFunction>(propName, boundedFunction, newArity);
            return Value(Function(std::make_shared<NativeCallable>(data)));
        }
    }

    return Value(Null{});
}

void ObjectData::set(const Value& key, const Value& value) {
    if (!isHashable(key)) {
        throw std::runtime_error("Không thể dùng key với kiểu dữ liệu này");
    }

    pairs[HashKey{key}] = value;
}

std::unique_ptr<Iterator> ObjectData::makeIterator() {
    return std::make_unique<ObjectIterator>(this);
}

ObjectIterator::ObjectIterator(ObjectData* data) {
    current = data->pairs.begin();
    end = data->pairs.end();
}

bool ObjectIterator::hasNext() const {
    return current != end;
}

Value ObjectIterator::next() {
    auto obj = std::make_shared<ObjectData>();
    obj->set(Value("first"), current->first.value);
    obj->set(Value("second"), current->second);
    ++current;
    return Value(obj);
}

std::string ObjectData::toString() const {
    std::ostringstream os;
    os << "{";
    if (pairs.empty()) {
        os << "}";
        return os.str();
    }
    bool first = true;
    for (const auto& pair : pairs) {
        if (!first) os << ", ";
        os << pair.first.value << ": " << pair.second;
        first = false;
    }
    os << "}";

    return os.str();
}

Value StringData::get(const Value& key) {
    if (std::holds_alternative<String>(key)) {
        const auto& propName = std::get<String>(key)->str;
        
        if (propName == "length") {
            return Value((Int)str.length());
        }

        auto it = stringLib->contents.find(propName);
        if (it != stringLib->contents.end()) {
            const auto& unboundFunction = std::get<Function>(it->second);
            auto self_shared = this->shared_from_this();
            NativeFnAdvanced boundedFunction = [self_shared, unboundFunction](Interpreter* engine, Arguments args) -> Value {
                std::vector<Value> finalArgs;
                finalArgs.push_back(Value(self_shared)); // Value(Array) from shared_ptr<ArrayData>
                finalArgs.insert(finalArgs.end(), args.begin(), args.end());
                return unboundFunction->call(engine, finalArgs);
            };
            Arity newArity = unboundFunction->arity();
            newArity.required = std::max(0, newArity.required - 1);
            auto data = std::make_shared<NativeFunction>(propName, boundedFunction, newArity);
            return Value(Function(std::make_shared<NativeCallable>(data)));
        }
    }

    if (std::holds_alternative<Int>(key)) {
        Int idx = std::get<Int>(key);
        if (static_cast<size_t>(idx) >= str.size() || idx < 0) {
            throw std::runtime_error("Chỉ số không nằm trong phạm vi truy cập phần tử!");
        }
        return Value(std::make_shared<StringData>(1, str[idx]));
    }
    
    throw std::runtime_error("Không thể truy cập chuỗi bằng key '" + ::toString(key) + "'.");
}

void StringData::set(const Value&, const Value&) {
    throw std::runtime_error("Không thể gán giá trị cho ký tự của chuỗi.");
}

std::unique_ptr<Iterator> StringData::makeIterator() {
    return std::make_unique<StringIterator>(this);
}

StringIterator::StringIterator(StringData* data) : strData(data) {}

bool StringIterator::hasNext() const { 
    return current < strData->str.size(); 
}

Value StringIterator::next() { 
    return std::make_shared<StringData>(1, char(strData->str[current++])); 
}

std::string StringData::toString() const {
    return str;
}

std::ostream& operator<<(std::ostream& os, const Value& v) {
    visit(overloaded {
        [&os](Null) {
            os << "null"; 
        },
        [&os](Int value) { 
            os << value; 
        },
        [&os](Real value) { 
            if (std::isnan(value)) {
                os << "NaN";
            } else if (std::isinf(value)) {
                os << ((value > 0) ? "Infinity" : "-Infinity");
            } else {
                os << value;
            }
        },
        [&os](Bool value) { 
            os << (value ? "true" : "false"); 
        },
        [&os](const Function&) { 
            os << "[function]"; 
        },
        [&os](const auto& val) {
            if (Stringifiable* str = toStringifiable(val)) {
                os << str->toString();
            } else {
                os << "[unprintable]";
            }
        }
    }, v);
    return os;
}

Str toString(const Value& v) {
    return visit(overloaded {
        [](Null) -> Str { 
            return "null"; 
        },
        [](Int value) -> Str { 
            return std::to_string(value); 
        },
        [](Real value) -> Str { 
            if (std::isnan(value)) {
                return "NaN";
            }
            if (std::isinf(value)) {
                return (value > 0) ? "Infinity" : "-Infinity";
            }
            return std::to_string(value);
        },
        [](Bool value) -> Str { 
            return (value ? "true" : "false"); 
        },
        [](const Function&) -> Str {
            return "[function]";
        },
        [](const auto& val) -> Str {
            if (Stringifiable* str = toStringifiable(val)) {
                return str->toString();
            } else {
                return "[unprintable]";
            }
        }
    }, v);
}