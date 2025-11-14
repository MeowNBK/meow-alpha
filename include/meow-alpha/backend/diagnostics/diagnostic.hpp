#pragma once

#include "common/token.hpp"
#include "utils/ansi_colors.hpp"

#include <stdexcept>
#include <string>
#include <format>
#include <unordered_map>
#include <iostream>
#include <vector>

enum class DiagnosticType {
    General,
    SyntaxError,
    SemanticError,
    RuntimeError,
    LogicError,
    ParseError,
    InterpretError
};

enum class Severity {
    Debug,
    Info,
    Warning,
    Error,
    FatalError
};

namespace std {
    template<>
    struct hash<DiagnosticType> {
        size_t operator()(const DiagnosticType& type) const {
            return static_cast<size_t>(type);
        }
    };

    template<>
    struct hash<Severity> {
        size_t operator()(const Severity& type) const {
            return static_cast<size_t>(type);
        }
    };
}

class Diagnostic : public std::runtime_error {
public:
    Diagnostic(DiagnosticType type, Severity sev, const std::string& msg, const Token &tok);

    std::string str() const noexcept;

    void pushCallSite(const Token& tok) {
        callStack_.push_back(tok);
    }
    Diagnostic withCallSite(const Token& tok) const;

    DiagnosticType getType() const noexcept { return type_; }
    Severity getSeverity() const noexcept { return severity_; }
    const Token& getToken() const noexcept { return token_; }
    const std::string& getMessage() const noexcept { return message_; }
    const std::vector<Token>& getCallStack() const noexcept { return callStack_; }

    static Diagnostic SyntaxErr(std::string msg, const Token &tok) { return { DiagnosticType::SyntaxError, Severity::Error, std::move(msg), tok }; }
    static Diagnostic SemanticErr(std::string msg, const Token &tok) { return { DiagnosticType::SemanticError, Severity::Error, std::move(msg), tok }; }
    static Diagnostic RuntimeErr(std::string msg, const Token &tok) { return { DiagnosticType::RuntimeError, Severity::Error, std::move(msg), tok }; }
    static Diagnostic LogicErr(std::string msg, const Token &tok) { return { DiagnosticType::LogicError, Severity::Error, std::move(msg), tok }; }
    static Diagnostic ParseErr(std::string msg, const Token &tok) { return { DiagnosticType::ParseError, Severity::Error, std::move(msg), tok }; }
    static Diagnostic InterpretErr(std::string msg, const Token &tok) { return { DiagnosticType::InterpretError, Severity::Error, std::move(msg), tok }; }
    static Diagnostic FatalErr(std::string msg, const Token &tok) { return { DiagnosticType::General, Severity::FatalError, std::move(msg), tok }; }

    static Diagnostic Warning(std::string msg, const Token &tok, DiagnosticType type = DiagnosticType::General) {
        return { type, Severity::Warning, std::move(msg), tok };
    }
    static Diagnostic Info(std::string msg, const Token &tok, DiagnosticType type = DiagnosticType::General) {
        return { type, Severity::Info, std::move(msg), tok };
    }
    static Diagnostic Debug(std::string msg, const Token &tok, DiagnosticType type = DiagnosticType::General) {
        return { type, Severity::Debug, std::move(msg), tok };
    }

private:
    DiagnosticType  type_;
    Severity        severity_;
    Token           token_;
    std::string     message_;
    std::vector<Token> callStack_;

    static const std::unordered_map<DiagnosticType, std::string_view> typeLabels;
    static const std::unordered_map<Severity, std::string_view>      sevColors;
    static const std::unordered_map<Severity, std::string_view>      sevLabels;
};

struct {
    void log(const std::string& str) {
        std::cout << "[LOG] " << str << "\n";
    }
    void error(const std::string& str) {
        std::cout << "[ERROR] " << str << "\n";
    }
    void debug(const std::string& str) {
        std::cout << "[DEBUG] " << str << "\n";
    }
} console;
