#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <string_view>
#include <memory>

class SourceFile {
    std::string filename, buffer;
    std::vector<size_t> offsets;

public:
    SourceFile(const std::string& path);

    SourceFile(const std::string& source, const std::string& path);

    std::string line(size_t n) const;

    const std::string& name() const;

    const std::string& getBuffer() const;
};

using SrcFilePtr = std::shared_ptr<SourceFile>;