#include "common/source_file.hpp"
#include "diagnostics/diagnostic.hpp"

SourceFile::SourceFile(const std::string& path) : filename(path), offsets({0}) {
    std::ifstream fin(path, std::ios::binary);
    if (!fin) {
        Token token(TokenType::END_OF_FILE, "", path, 0, 0, nullptr);
        throw Diagnostic(DiagnosticType::General, Severity::FatalError, "Khét lẹt luôn, hông tìm thấy file code. Chịu rồi bạn!", token);
    }
    buffer.assign(std::istreambuf_iterator<char>(fin.rdbuf()), {});
    for (size_t i = 0; i < buffer.size(); ++i) {
        if (buffer[i] == '\n') {
            offsets.push_back(i + 1);
        }
    }
}

SourceFile::SourceFile(const std::string& source, const std::string& path): buffer(source), filename(path), offsets({0}) {
    for (size_t i = 0; i < buffer.size(); ++i) {
        if (buffer[i] == '\n') {
            offsets.push_back(i + 1);
        } 
    }
}

std::string SourceFile::line(size_t n) const {
    if (n == 0 || n > offsets.size()) return {};
    size_t s = offsets[n - 1];
    size_t e = (n == offsets.size() ? buffer.size() : offsets[n]);
    if (e > s && buffer[e - 1] == '\n') --e;
    return { buffer.data() + s, e - s };
}

const std::string& SourceFile::name() const {
    return filename;
}

const std::string& SourceFile::getBuffer() const { 
    return buffer; 
}