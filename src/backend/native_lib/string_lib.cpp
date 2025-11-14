#include "native_lib/standard_lib.hpp"
#include "diagnostics/meow_exceptions.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>

static inline std::string valToStdStr(const Value& v) {
    return std::get<String>(v)->str;
}

Value native_string_split(Arguments args) {
    const std::string& str = valToStdStr(args[0]);
    std::string delimiter = " ";
    if (args.size() > 1) delimiter = valToStdStr(args[1]);

    auto result = std::make_shared<ArrayData>();
    size_t start = 0, end;
    while ((end = str.find(delimiter, start)) != std::string::npos) {
        result->elements.emplace_back(str.substr(start, end - start));
        start = end + delimiter.length();
    }
    result->elements.emplace_back(str.substr(start));
    return Value(result);
}

Value native_string_join(Arguments args) {
    const std::string& sep = valToStdStr(args[0]);
    const auto& arr = std::get<Array>(args[1]);

    std::ostringstream os;
    for (size_t i = 0; i < arr->elements.size(); ++i) {
        os << toString(arr->elements[i]);
        if (i < arr->elements.size() - 1) os << sep;
    }
    return Value(os.str());
}

Value native_string_upper(Arguments args) {
    std::string str = valToStdStr(args[0]);
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return Value(str);
}

Value native_string_lower(Arguments args) {
    std::string str = valToStdStr(args[0]);
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return Value(str);
}

Value native_string_trim(Arguments args) {
    std::string str = valToStdStr(args[0]);
    str.erase(std::find_if(str.rbegin(), str.rend(),
        [](unsigned char ch) { return !std::isspace(ch); }).base(), str.end());
    str.erase(str.begin(), std::find_if(str.begin(), str.end(),
        [](unsigned char ch) { return !std::isspace(ch); }));
    return Value(str);
}

Value native_string_startsWith(Arguments args) {
    const std::string& str = valToStdStr(args[0]);
    const std::string& prefix = valToStdStr(args[1]);
    return Value(str.rfind(prefix, 0) == 0);
}

Value native_string_endsWith(Arguments args) {
    const std::string& str = valToStdStr(args[0]);
    const std::string& suffix = valToStdStr(args[1]);
    if (suffix.size() > str.size()) return Value(false);
    return Value(std::equal(suffix.rbegin(), suffix.rend(), str.rbegin()));
}

Value native_string_replace(Arguments args) {
    std::string str = valToStdStr(args[0]);
    const std::string& from = valToStdStr(args[1]);
    const std::string& to = valToStdStr(args[2]);
    size_t pos = str.find(from);
    if (pos != std::string::npos) {
        str.replace(pos, from.size(), to);
    }
    return Value(str);
}

Value native_string_contains(Arguments args) {
    const std::string& str = valToStdStr(args[0]);
    const std::string& sub = valToStdStr(args[1]);
    return Value(str.find(sub) != std::string::npos);
}

Value native_string_indexOf(Arguments args) {
    const std::string& str = valToStdStr(args[0]);
    const std::string& sub = valToStdStr(args[1]);
    size_t start = 0;
    if (args.size() > 2) start = (size_t)std::get<Int>(args[2]);
    size_t pos = str.find(sub, start);
    return Value((pos == std::string::npos) ? (Int)-1 : (Int)pos);
}

Value native_string_lastIndexOf(Arguments args) {
    const std::string& str = valToStdStr(args[0]);
    const std::string& sub = valToStdStr(args[1]);
    size_t pos = str.rfind(sub);
    return Value((pos == std::string::npos) ? (Int)-1 : (Int)pos);
}

Value native_string_substring(Arguments args) {
    const std::string& str = valToStdStr(args[0]);
    size_t start = (size_t)std::get<Int>(args[1]);
    size_t len = str.size() - start;
    if (args.size() > 2) len = (size_t)std::get<Int>(args[2]);
    if (start > str.size()) return Value("");
    return Value(str.substr(start, len));
}

Value native_string_slice(Arguments args) {
    const std::string& str = valToStdStr(args[0]);
    int start = (int)std::get<Int>(args[1]);
    int end = (args.size() > 2) ? (int)std::get<Int>(args[2]) : (int)str.size();
    if (start < 0) start += (int)str.size();
    if (end < 0) end += (int)str.size();
    if (start < 0) start = 0;
    if (end > (int)str.size()) end = (int)str.size();
    if (start >= end) return Value("");
    return Value(str.substr(start, end - start));
}

Value native_string_repeat(Arguments args) {
    const std::string& str = valToStdStr(args[0]);
    int count = (int)std::get<Int>(args[1]);
    if (count <= 0) return Value("");
    std::string result;
    result.reserve(str.size() * count);
    for (int i = 0; i < count; ++i) result += str;
    return Value(result);
}

Value native_string_padLeft(Arguments args) {
    std::string str = valToStdStr(args[0]);
    size_t length = (size_t)std::get<Int>(args[1]);
    char ch = ' ';
    if (args.size() > 2) ch = valToStdStr(args[2])[0];
    if (str.size() < length) {
        str.insert(str.begin(), length - str.size(), ch);
    }
    return Value(str);
}

Value native_string_padRight(Arguments args) {
    std::string str = valToStdStr(args[0]);
    size_t length = (size_t)std::get<Int>(args[1]);
    char ch = ' ';
    if (args.size() > 2) ch = valToStdStr(args[2])[0];
    if (str.size() < length) {
        str.append(length - str.size(), ch);
    }
    return Value(str);
}

Value native_string_equalsIgnoreCase(Arguments args) {
    std::string a = valToStdStr(args[0]);
    std::string b = valToStdStr(args[1]);
    std::transform(a.begin(), a.end(), a.begin(), ::tolower);
    std::transform(b.begin(), b.end(), b.begin(), ::tolower);
    return Value(a == b);
}

Value native_string_charAt(Arguments args) {
    const std::string& str = valToStdStr(args[0]);
    size_t idx = (size_t)std::get<Int>(args[1]);
    if (idx >= str.size()) return Value("");
    return Value(std::string(1, str[idx]));
}

Value native_string_charCodeAt(Arguments args) {
    const std::string& str = valToStdStr(args[0]);
    size_t idx = (size_t)std::get<Int>(args[1]);
    if (idx >= str.size()) return Value(Int(-1));
    return Value(Int((unsigned char)str[idx]));
}
Value native_string_size(Arguments args) {
    auto& tstring = std::get<String>(args[0]);
    return Value((Int)tstring->str.size());
}

StringLib::StringLib() {
    registerFn("split", native_string_split, Arity::range(1, 1));
    registerFn("join", native_string_join, 2);
    registerFn("upper", native_string_upper, 1);
    registerFn("lower", native_string_lower, 1);
    registerFn("trim", native_string_trim, 1);

    registerFn("startsWith", native_string_startsWith, 2);
    registerFn("endsWith", native_string_endsWith, 2);
    registerFn("replace", native_string_replace, 3);
    registerFn("contains", native_string_contains, 2);
    registerFn("indexOf", native_string_indexOf, Arity::range(2, 1));
    registerFn("lastIndexOf", native_string_lastIndexOf, 2);

    registerFn("substring", native_string_substring, Arity::range(2, 1));
    registerFn("slice", native_string_slice, Arity::range(2, 1));
    registerFn("repeat", native_string_repeat, 2);

    registerFn("padLeft", native_string_padLeft, Arity::range(2, 1));
    registerFn("padRight", native_string_padRight, Arity::range(2, 1));
    registerFn("equalsIgnoreCase", native_string_equalsIgnoreCase, 2);

    registerFn("charAt", native_string_charAt, 2);
    registerFn("charCodeAt", native_string_charCodeAt, 2);

    registerFn("size", native_string_size, 1);

}
