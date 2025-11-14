#include "module/module_manager.hpp"
#include "runtime/value.hpp"
#include <sstream>
#include <fstream>
#include <filesystem>

#include "common/source_file.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "visitor/tree_walker.hpp"

#include <cstdlib>   // std::getenv
#include <vector>
#include <string>
#include <algorithm>

#ifdef _WIN32
static constexpr char PATH_SEP = ';';
#else
static constexpr char PATH_SEP = ':';
#endif

// include paths chung cho toàn file (không phải member của class)
static std::vector<std::filesystem::path> g_includePaths;

static std::vector<std::string> splitEnvPaths(const std::string& s) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == PATH_SEP) {
            if (!cur.empty()) { out.push_back(cur); cur.clear(); }
        } else cur.push_back(c);
    }
    if (!cur.empty()) out.push_back(cur);
    return out;
}
// --- kết thúc bổ sung ---


/* --- Thay constructor mặc định --- */
ModuleManager::ModuleManager() {
    nativeModules.emplace("io", std::make_unique<NativeLibrary>(IoLib()));
    nativeModules.emplace("math", std::make_unique<NativeLibrary>(MathLib()));
    nativeModules.emplace("array", std::make_unique<NativeLibrary>(ArrayLib()));
    nativeModules.emplace("object", std::make_unique<NativeLibrary>(ObjectLib()));
    nativeModules.emplace("string", std::make_unique<NativeLibrary>(StringLib()));
    nativeModules.emplace("time", std::make_unique<NativeLibrary>(TimeLib()));
    nativeModules.emplace("system", std::make_unique<NativeLibrary>(SystemLib()));
    nativeModules.emplace("random", std::make_unique<NativeLibrary>(RandomLib()));
    nativeModules.emplace("json", std::make_unique<NativeLibrary>(JsonLib()));
    nativeModules.emplace("meta", std::make_unique<NativeLibrary>(MetaLib()));

    initNativeMethods(
        *nativeModules["array"],
        *nativeModules["object"],
        *nativeModules["string"]
    );

    // Parse MODULE_PATH env (nếu có)
    if (const char* mp = std::getenv("MODULE_PATH")) {
        auto parts = splitEnvPaths(std::string(mp));
        for (auto &p : parts) {
            if (!p.empty()) g_includePaths.emplace_back(p);
        }
    }
}

/* --- Thay constructor argc/argv: delegate vào ctor mặc định rồi parse -I --- */
ModuleManager::ModuleManager(int argc, char* arguments[]) : ModuleManager() {
    for (int i = 0; i < argc; ++i) {
        if (arguments[i] != nullptr) {
            argv.emplace_back(arguments[i]);
        }
    }

    // Parse CLI -I / --include (ví dụ: -Ipath hoặc -I path, --include path)
    for (int i = 0; i < argc; ++i) {
        std::string a = arguments[i] ? arguments[i] : "";
        if ((a == "-I" || a == "--include") && i + 1 < argc && arguments[i+1]) {
            g_includePaths.emplace_back(arguments[++i]);
        } else if (a.rfind("-I", 0) == 0 && a.size() > 2) { // -Ipath
            g_includePaths.emplace_back(a.substr(2));
        }
    }
}

Value ModuleManager::load(const std::string& importerPath, const std::string& importPath) {
    if (nativeModules.count(importPath)) {
        if (moduleCache.count(importPath)) {
            return moduleCache.at(importPath).exports;
        }

        const auto& library = nativeModules.at(importPath);
        auto exportsObj = std::make_shared<ObjectData>();
        for(const auto& pair : library->contents) {
            exportsObj->pairs[HashKey{(Value(pair.first))}] = pair.second;
        }
        Value exportsValue = Value(Object(exportsObj));
        
        moduleCache[importPath] = ParsedModule{nullptr, exportsValue};
        return exportsValue;
    }

    std::filesystem::path finalPath;
    bool found = false;
    std::error_code ec;

    // Tạo danh sách candidate theo thứ tự ưu tiên:
    // 1) importer dir
    // 2) include paths (g_includePaths)
    // 3) cwd
    // 4) original importPath
    std::vector<std::filesystem::path> candidates;

    if (!importerPath.empty()) {
        candidates.push_back(std::filesystem::path(importerPath).parent_path() / importPath);
    }

    for (const auto &sp : g_includePaths) {
        candidates.push_back(sp / importPath);
    }

    candidates.push_back(std::filesystem::current_path() / importPath);
    candidates.push_back(std::filesystem::path(importPath));

    // Thử từng candidate: tồn tại & là file -> canonical hoặc absolute
    for (const auto& cand : candidates) {
        if (std::filesystem::exists(cand, ec) && !ec && std::filesystem::is_regular_file(cand, ec) && !ec) {
            try {
                finalPath = std::filesystem::canonical(cand);
            } catch (const std::filesystem::filesystem_error&) {
                finalPath = std::filesystem::absolute(cand);
            }
            found = true;
            break;
        }
        // thử thêm .meow nếu người dùng quên extension
        std::filesystem::path candWithExt = cand;
        if (candWithExt.extension().empty()) candWithExt += ".meow";
        if (std::filesystem::exists(candWithExt, ec) && !ec && std::filesystem::is_regular_file(candWithExt, ec) && !ec) {
            try {
                finalPath = std::filesystem::canonical(candWithExt);
            } catch (const std::filesystem::filesystem_error&) {
                finalPath = std::filesystem::absolute(candWithExt);
            }
            found = true;
            break;
        }
    }

    if (!found) {
        std::cerr << "Không tìm thấy module: '" << importPath << "' (đã thử importer dir, include paths, cwd và đường dẫn gốc).\n";
        throw std::runtime_error("Module không tồn tại: " + importPath);
    }

    std::string canonicalPath = finalPath.string();

    if (moduleCache.count(canonicalPath)) {
        return moduleCache.at(canonicalPath).exports;
    }

    SrcFilePtr srcFile = std::make_unique<SourceFile>(canonicalPath);
    Lexer lexer(srcFile);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parseProgram();

    ParsedModule astModule;
    astModule.ast = std::move(program);
    astModule.exports = Value(Object(std::make_shared<ObjectData>()));
    
    auto& cachedModule = (moduleCache[canonicalPath] = std::move(astModule));

    TreeWalker moduleWalker(this, srcFile, &cachedModule.exports, argv);
    moduleWalker.isModuleContext = true;
    moduleWalker.visit(cachedModule.ast.get());
    
    return cachedModule.exports;
}


Value ModuleManager::loadFromSource(const std::string& moduleKey, const std::string& sourceCode) {
    if (moduleCache.count(moduleKey)) {
        return moduleCache.at(moduleKey).exports;
    }

    SrcFilePtr srcFile = std::make_unique<SourceFile>(sourceCode); 
    Lexer lexer(srcFile);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parseProgram();

    ParsedModule astModule;
    astModule.ast = std::move(program);
    astModule.exports = Value(Object(std::make_shared<ObjectData>()));

    auto& cachedModule = (moduleCache[moduleKey] = std::move(astModule));

    TreeWalker moduleWalker(this, srcFile, &cachedModule.exports, argv.empty() ? std::vector<std::string>{} : argv);
    moduleWalker.visit(cachedModule.ast.get());

    return cachedModule.exports;
}
