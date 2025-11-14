#pragma once

#include "native_lib/standard_lib.hpp"
#include "runtime/value.hpp"
#include <string>
#include <unordered_map>

class Interpreter;
struct Program;

struct ParsedModule {
    std::unique_ptr<Program> ast;
    Value exports;
};

class ModuleManager {
    std::unordered_map<std::string, std::unique_ptr<NativeLibrary>> nativeModules;
    std::vector<std::string> argv;
public:
    ModuleManager();
    ModuleManager(int argc, char* argv[]);

    std::unordered_map<std::string, ParsedModule> moduleCache;

    Value load(const std::string& importerPath, const std::string& importPath);
    Value loadFromSource(const std::string& moduleKey, const std::string& sourceCode);
};