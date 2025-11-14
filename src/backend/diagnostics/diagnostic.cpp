#include "diagnostics/diagnostic.hpp"
#include <filesystem>
namespace fs = std::filesystem;

static std::string shortenPath(const std::string& path) {
    try {
        fs::path full = fs::absolute(path);
        fs::path cwd = fs::current_path();

        auto rel = fs::relative(full, cwd);
        if (!rel.empty() && rel.native().size() < full.native().size()) {
            return rel.string();
        }
        return full.string();
    } catch (...) {
        return path;
    }
}

const std::unordered_map<DiagnosticType, std::string_view> Diagnostic::typeLabels = {
    { DiagnosticType::General,          "Chung"           },
    { DiagnosticType::SyntaxError,      "Lỗi cú pháp"      },
    { DiagnosticType::SemanticError,    "Lỗi ngữ nghĩa"    },
    { DiagnosticType::RuntimeError,     "Lỗi runtime"     },
    { DiagnosticType::LogicError,       "Lỗi Logic"       },
    { DiagnosticType::ParseError,       "Lỗi phân tích"   },
    { DiagnosticType::InterpretError,   "Lỗi thực thi"    }
};

const std::unordered_map<Severity, std::string_view> Diagnostic::sevColors = {
    { Severity::Debug,      AnsiColors::BRIGHT_CYAN     },
    { Severity::Info,       AnsiColors::BRIGHT_BLUE     },
    { Severity::Warning,    AnsiColors::BRIGHT_YELLOW   },
    { Severity::Error,      AnsiColors::BRIGHT_RED      },
    { Severity::FatalError, AnsiColors::RED             }
};

const std::unordered_map<Severity, std::string_view> Diagnostic::sevLabels = {
    { Severity::Debug,      "DEBUG"           },
    { Severity::Info,       "THÔNG TIN"       },
    { Severity::Warning,    "CẢNH BÁO"        },
    { Severity::Error,      "LỖI"             },
    { Severity::FatalError, "LỖI NGHIÊM TRỌNG"}
};

Diagnostic::Diagnostic(DiagnosticType type, Severity sev, const std::string &msg, const Token &tok)
    : type_(type), severity_(sev), token_(tok), message_(msg), std::runtime_error(msg) {}

static std::string formatToken(const Token& token, std::string_view sevColor, std::string_view sevLabel, std::string_view typeLabel, const std::string& message) {
    std::string header = std::format(
        "{}{}:{}:{}{} {}{}[{}] {}{}{}{}: {}{}{}",
        AnsiColors::BOLD, shortenPath(token.filename), token.line, token.col, AnsiColors::RESET,
        sevColor, AnsiColors::BOLD, typeLabel, AnsiColors::RESET,
        AnsiColors::BOLD, sevColor, sevLabel, AnsiColors::RESET,
        sevColor, message, AnsiColors::RESET
    );

    std::string codeLine = token.getLine();
    size_t pos = (token.col > 1) ? token.col - 1 : 0;
    pos = std::min(pos, codeLine.size());

    std::string indicator(pos, ' ');
    indicator.append(std::max<size_t>(1, token.lexeme.length()), '^');

    return std::format(
        "{}\n  {}-> {}{}{}\n  {}  {}{}{}",
        header,
        sevColor, AnsiColors::BOLD, codeLine, AnsiColors::RESET,
        sevColor, std::string(pos, ' '), std::string(token.lexeme.length(), '^'), AnsiColors::RESET
    );
}

std::string Diagnostic::str() const noexcept {
    try {
        std::string_view sevColor = sevColors.at(severity_);
        std::string_view sevLabel = sevLabels.at(severity_);
        std::string_view typeLabel = typeLabels.at(type_);

        std::string out = formatToken(token_, sevColor, sevLabel, typeLabel, message_);

        for (auto it = callStack_.rbegin(); it != callStack_.rend(); ++it) {
            out += std::format(
                "\n\n{}Gọi từ {}:{}:{}{}\n  {}-> {}{}{}\n  {}  {}{}{}",
                sevColor, it->filename, it->line, it->col, AnsiColors::RESET,
                sevColor, AnsiColors::BOLD, it->getLine(), AnsiColors::RESET,
                sevColor, std::string(it->col - 1, ' '), "^", AnsiColors::RESET
            );
        }

        return out;
    } catch (...) {
        return "Lỗi định dạng: " + message_ + " ở " + token_.filename + ":" + std::to_string(token_.line);
    }
}

Diagnostic Diagnostic::withCallSite(const Token& tok) const {
    Diagnostic copy = *this;
    copy.callStack_.push_back(tok);
    return copy;
}