// json_lib.cpp
#include "runtime/value.hpp"
#include "diagnostics/meow_exceptions.hpp"
#include "native_lib/standard_lib.hpp"

#include <stdexcept>
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <cctype>
#include <iomanip>
#include <limits>

using namespace std;

// ---------- JSON Parser ----------

class JsonParser {
private:
    std::string json;
    size_t pos = 0;

    char peek() const {
        return pos < json.size() ? json[pos] : '\0';
    }

    void advance() { if (pos < json.size()) ++pos; }

    void skipWhitespace() {
        while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) pos++;
    }

    void expectChar(char c) {
        skipWhitespace();
        if (peek() != c) {
            std::ostringstream oss;
            oss << "Expected '" << c << "' but found '" << (peek() ? peek() : '?') << "'";
            throw std::runtime_error(oss.str());
        }
        advance();
    }

    Value parseValue() {
        skipWhitespace();
        if (pos >= json.size()) throw std::runtime_error("Unexpected end of JSON string");
        char c = peek();
        if (c == '{') return parseObject();
        if (c == '[') return parseArray();
        if (c == '"') return parseString();
        if (c == 't') return parseTrue();
        if (c == 'f') return parseFalse();
        if (c == 'n') return parseNull();
        if ((c >= '0' && c <= '9') || c == '-') return parseNumber();

        std::ostringstream oss;
        oss << "Unexpected character in JSON: '" << c << "'";
        throw std::runtime_error(oss.str());
    }

    Value parseObject() {
        expectChar('{');
        skipWhitespace();
        auto objData = std::make_shared<ObjectData>();
        skipWhitespace();
        if (peek() == '}') { advance(); return Value(objData); }

        while (true) {
            skipWhitespace();
            if (peek() != '"') throw std::runtime_error("Expected string for object key");
            Value key = parseString(); // JSON keys must be strings
            skipWhitespace();
            if (peek() != ':') throw std::runtime_error("Expected ':' after object key");
            advance(); // consume ':'
            Value val = parseValue();
            objData->pairs[HashKey{key}] = val;
            skipWhitespace();
            if (peek() == '}') { advance(); break; }
            if (peek() == ',') { advance(); continue; }
            throw std::runtime_error("Expected ',' or '}' in object");
        }
        return Value(objData);
    }

    Value parseArray() {
        expectChar('[');
        skipWhitespace();
        auto arrData = std::make_shared<ArrayData>();
        if (peek() == ']') { advance(); return Value(arrData); }

        while (true) {
            Value el = parseValue();
            arrData->elements.push_back(el);
            skipWhitespace();
            if (peek() == ']') { advance(); break; }
            if (peek() == ',') { advance(); continue; }
            throw std::runtime_error("Expected ',' or ']' in array");
        }
        return Value(arrData);
    }

    // Parse JSON string, handle common escapes (\" \\ \/ \b \f \n \r \t and \uXXXX)
    Value parseString() {
        expectChar('"'); // consume opening quote
        std::string out;
        while (pos < json.size()) {
            char c = peek();
            if (c == '"') { advance(); break; } // closing quote
            if (c == '\\') {
                advance(); // skip '\'
                if (pos >= json.size()) throw std::runtime_error("Invalid escape at end of string");
                char esc = peek();
                switch (esc) {
                    case '"': out.push_back('"'); break;
                    case '\\': out.push_back('\\'); break;
                    case '/': out.push_back('/'); break;
                    case 'b': out.push_back('\b'); break;
                    case 'f': out.push_back('\f'); break;
                    case 'n': out.push_back('\n'); break;
                    case 'r': out.push_back('\r'); break;
                    case 't': out.push_back('\t'); break;
                    case 'u': {
                        // parse \uXXXX unicode escape -> produce UTF-8
                        advance(); // skip 'u'
                        if (pos + 4 > json.size()) throw std::runtime_error("Invalid \\u escape");
                        unsigned codepoint = 0;
                        for (int i = 0; i < 4; ++i) {
                            char h = peek();
                            advance();
                            codepoint <<= 4;
                            if (h >= '0' && h <= '9') codepoint |= (h - '0');
                            else if (h >= 'a' && h <= 'f') codepoint |= (10 + h - 'a');
                            else if (h >= 'A' && h <= 'F') codepoint |= (10 + h - 'A');
                            else throw std::runtime_error("Invalid hex in \\u escape");
                        }
                        // encode codepoint to UTF-8
                        if (codepoint <= 0x7F) {
                            out.push_back(static_cast<char>(codepoint));
                        } else if (codepoint <= 0x7FF) {
                            out.push_back(static_cast<char>(0xC0 | ((codepoint >> 6) & 0x1F)));
                            out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
                        } else {
                            out.push_back(static_cast<char>(0xE0 | ((codepoint >> 12) & 0x0F)));
                            out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
                            out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
                        }
                        continue; // already advanced past hex digits
                    }
                    default:
                        // unknown escape -> just include literal
                        out.push_back(esc);
                }
                advance(); // move past escape char (or already advanced for 'u' but safe)
            } else {
                out.push_back(c);
                advance();
            }
        }
        return Value(out);
    }

    Value parseNumber() {
        size_t start = pos;
        if (peek() == '-') advance();
        if (peek() == '0') {
            advance();
        } else if (peek() >= '1' && peek() <= '9') {
            while (peek() >= '0' && peek() <= '9') advance();
        } else {
            throw std::runtime_error("Invalid number");
        }

        bool isFloat = false;
        if (peek() == '.') {
            isFloat = true;
            advance();
            if (!(peek() >= '0' && peek() <= '9')) throw std::runtime_error("Invalid fraction in number");
            while (peek() >= '0' && peek() <= '9') advance();
        }

        if (peek() == 'e' || peek() == 'E') {
            isFloat = true;
            advance();
            if (peek() == '+' || peek() == '-') advance();
            if (!(peek() >= '0' && peek() <= '9')) throw std::runtime_error("Invalid exponent in number");
            while (peek() >= '0' && peek() <= '9') advance();
        }

        std::string numStr = json.substr(start, pos - start);
        try {
            if (isFloat) {
                double d = std::stod(numStr);
                return Value(static_cast<Real>(d));
            } else {
                long long i = std::stoll(numStr);
                return Value(static_cast<Int>(i));
            }
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Invalid numeric value: ") + e.what());
        }
    }

    Value parseTrue() {
        if (json.substr(pos, 4) != "true") throw std::runtime_error("Invalid literal. Expected 'true'.");
        pos += 4;
        return Value(true);
    }

    Value parseFalse() {
        if (json.substr(pos, 5) != "false") throw std::runtime_error("Invalid literal. Expected 'false'.");
        pos += 5;
        return Value(false);
    }

    Value parseNull() {
        if (json.substr(pos, 4) != "null") throw std::runtime_error("Invalid literal. Expected 'null'.");
        pos += 4;
        return Value(Null{});
    }

public:
    Value parse(const Str& str) {
        json = str; // copy to own storage to avoid dangling view
        pos = 0;
        skipWhitespace();
        Value result = parseValue();
        skipWhitespace();
        if (pos != json.size()) {
            throw std::runtime_error("Extra characters after JSON document");
        }
        return result;
    }
};

// ---------- JSON Stringify ----------

static Str escapeJsonString(const Str& s) {
    std::ostringstream oss;
    oss << '"';
    for (unsigned char ch : s) {
        switch (ch) {
            case '\"': oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b";  break;
            case '\f': oss << "\\f";  break;
            case '\n': oss << "\\n";  break;
            case '\r': oss << "\\r";  break;
            case '\t': oss << "\\t";  break;
            default:
                if (ch < 0x20) {
                    // control characters -> \u00XX
                    oss << "\\u";
                    oss << std::hex << std::setw(4) << std::setfill('0') << (int)ch << std::dec;
                } else {
                    oss << ch;
                }
        }
    }
    oss << '"';
    return oss.str();
}

static Str toJsonRecursive(const Value& value, int indentLevel, int tabSize) {
    std::ostringstream ss;
    Str curIndent(indentLevel * tabSize, ' ');
    Str nextIndent((indentLevel + 1) * tabSize, ' ');

    visit([&](const auto& val) {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, Null>) {
            ss << "null";
        } else if constexpr (std::is_same_v<T, Int>) {
            ss << val;
        } else if constexpr (std::is_same_v<T, Real>) {
            // ensure consistent formatting for floats
            ss << std::setprecision(std::numeric_limits<Real>::digits10 + 1) << val;
        } else if constexpr (std::is_same_v<T, Bool>) {
            ss << (val ? "true" : "false");
        } else if constexpr (std::is_same_v<T, String>) {
            ss << escapeJsonString(val->str);
        } else if constexpr (std::is_same_v<T, Array>) {
            if (val->elements.empty()) {
                ss << "[]";
            } else {
                ss << "[\n";
                for (size_t i = 0; i < val->elements.size(); ++i) {
                    ss << nextIndent << toJsonRecursive(val->elements[i], indentLevel + 1, tabSize);
                    if (i + 1 < val->elements.size()) ss << ",";
                    ss << "\n";
                }
                ss << curIndent << "]";
            }
        } else if constexpr (std::is_same_v<T, Object>) {
            if (val->pairs.empty()) {
                ss << "{}";
            } else {
                ss << "{\n";
                size_t index = 0;
                for (const auto& pair : val->pairs) {
                    // pair.first is HashKey
                    ss << nextIndent << escapeJsonString(toString(pair.first.value));
                    ss << ": " << toJsonRecursive(pair.second, indentLevel + 1, tabSize);
                    if (index + 1 < val->pairs.size()) ss << ",";
                    ss << "\n";
                    ++index;
                }
                ss << curIndent << "}";
            }
        } else {
            ss << "\"<unsupported type>\"";
        }
    }, value);

    return ss.str();
}

// ---------- Exposed functions to the VM ----------

Value toJsonFn(Arguments args) {
    if (args.empty()) {
        throw FunctionException("Hàm 'toJson' cần ít nhất 1 tham số.");
    }
    const Value& valueToConvert = args[0];
    int indentLevel = 0;
    int tabSize = 2;

    if (args.size() > 1) {
        if (std::holds_alternative<Int>(args[1])) {
            indentLevel = static_cast<int>(std::get<Int>(args[1]));
        } else {
            throw FunctionException("Đối số thứ hai của 'toJson' phải là một số nguyên (indent level).");
        }
    }

    if (args.size() > 2) {
        if (std::holds_alternative<Int>(args[2])) {
            tabSize = static_cast<int>(std::get<Int>(args[2]));
        } else {
            throw FunctionException("Đối số thứ ba của 'toJson' phải là một số nguyên (tab size).");
        }
    }

    Str out = toJsonRecursive(valueToConvert, indentLevel, tabSize);
    return Value(out);
}

Value jsonToValue(Arguments args) {
    if (args.empty() || !std::holds_alternative<String>(args[0])) {
        throw FunctionException("Hàm 'jsonToValue' cần một tham số là chuỗi JSON.");
    }
    const Str& jsonString = std::get<String>(args[0])->str;

    try {
        JsonParser parser;
        return parser.parse(jsonString);
    } catch (const std::exception& e) {
        throw FunctionException("Lỗi cú pháp JSON: " + Str(e.what()));
    }
}

// ---------- Registration in native lib ----------

JsonLib::JsonLib() {
    registerFn("parse", jsonToValue, 1);
    // stringify alias -> toJson; accept 1..3 args
    registerFn("stringify", toJsonFn, Arity::range(1, 3));
}
