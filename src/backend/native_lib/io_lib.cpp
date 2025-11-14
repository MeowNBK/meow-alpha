#include "native_lib/standard_lib.hpp"
#include "diagnostics/meow_exceptions.hpp"
#include "runtime/value.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <chrono>

namespace fs = std::filesystem;



Value input(Arguments args) {
    if (!args.empty()) {
        std::cout << toString(args[0]);
    }
    std::string line;
    std::getline(std::cin, line);
    if (std::cin.fail() || std::cin.eof()) {
        return Value(Null{});
    }
    return Value(line);
}

Value readFile(Arguments args) {

    if (!std::holds_alternative<String>(args[0])) {
        throw FunctionException(
            "Hàm 'read' yêu cầu tham số đầu tiên phải là chuỗi (tên file).");
    }
    const std::string& path = std::get<String>(args[0])->str;

    std::ios_base::openmode mode = std::ios::in;
    if (args.size() == 2 && std::holds_alternative<Object>(args[1])) {
        Object opts = std::get<Object>(args[1]);
        auto it = opts->pairs.find(HashKey{ Value("mode") });
        if (it != opts->pairs.end() && std::holds_alternative<String>(it->second)) {
            if (std::get<String>(it->second)->str == "binary")
                mode |= std::ios::binary;
        }
    }

    std::ifstream fin(path, mode);
    if (!fin.is_open()) {
        throw FunctionException("Không thể mở file '" + path + "' để đọc.");
    }

    std::ostringstream buf;
    buf << fin.rdbuf();
    fin.close();

    return Value(buf.str());
}

Value writeFile(Arguments args) {

    if (!std::holds_alternative<String>(args[0])) {
        throw FunctionException(
            "Hàm 'write' yêu cầu tham số đầu tiên phải là chuỗi (tên file).");
    }
    const std::string& path = std::get<String>(args[0])->str;
    

    const Value& content = args[1];

    std::ios_base::openmode mode = std::ios::out;
    

    if (args.size() == 3 && std::holds_alternative<Object>(args[2])) {
        Object opts = std::get<Object>(args[2]);

        auto app_it = opts->pairs.find(HashKey{ Value("append") });
        if (app_it != opts->pairs.end() &&
            std::holds_alternative<Bool>(app_it->second) &&
            std::get<Bool>(app_it->second))
        {
            mode |= std::ios::app;
        }

        auto bin_it = opts->pairs.find(HashKey{ Value("mode") });
        if (bin_it != opts->pairs.end() &&
            std::holds_alternative<String>(bin_it->second) &&
            std::get<String>(bin_it->second)->str == "binary")
        {
            mode |= std::ios::binary;
        }
    }

    std::ofstream fout(path, mode);
    if (!fout.is_open()) {
        throw FunctionException("Không thể mở file '" + path + "' để ghi.");
    }

    if ((mode & std::ios::binary) != 0) {

        const std::string& s = toString(content);
        fout.write(s.data(), s.size());
    } else {

        fout << toString(content);
    }
    fout.close();

    return Value(Null{});
}

Value fileExists(Arguments args) {
    if (!std::holds_alternative<String>(args[0])) {
        throw FunctionException("Hàm 'fileExists' yêu cầu tham số là chuỗi đường dẫn.");
    }
    return Value(fs::exists(std::get<String>(args[0])->str));
}

Value isDirectory(Arguments args) {
    if (!std::holds_alternative<String>(args[0])) {
        throw FunctionException("Hàm 'isDirectory' yêu cầu tham số là chuỗi đường dẫn.");
    }
    return Value(fs::is_directory(std::get<String>(args[0])->str));
}

Value listDir(Arguments args) {
    if (!std::holds_alternative<String>(args[0])) {
        throw FunctionException("Hàm 'listDir' yêu cầu tham số là chuỗi đường dẫn.");
    }
    const std::string& path_str = std::get<String>(args[0])->str;
    auto arrData = std::make_shared<ArrayData>();
    try {
        for (const auto& entry : fs::directory_iterator(path_str)) {
            arrData->elements.push_back(Value(entry.path().string()));
        }
    } catch(const fs::filesystem_error& e) {
        throw FunctionException("Lỗi khi liệt kê thư mục '" + path_str + "': " + e.what());
    }
    return Value(arrData);
}

Value createDir(Arguments args) {
    if (!std::holds_alternative<String>(args[0])) {
        throw FunctionException("Hàm 'createDir' yêu cầu tham số là chuỗi đường dẫn.");
    }
    const std::string& path_str = std::get<String>(args[0])->str;
    try {
        return Value(fs::create_directories(path_str));
    } catch(const fs::filesystem_error& e) {
        throw FunctionException("Lỗi khi tạo thư mục '" + path_str + "': " + e.what());
    }
}

Value deleteFile(Arguments args) {
     if (!std::holds_alternative<String>(args[0])) {
        throw FunctionException("Hàm 'deleteFile' yêu cầu tham số là chuỗi đường dẫn.");
    }
    const std::string& path_str = std::get<String>(args[0])->str;
    try {
        return Value(fs::remove(path_str));
    } catch(const fs::filesystem_error& e) {
        throw FunctionException("Lỗi khi xóa file '" + path_str + "': " + e.what());
    }
}

Value getFileTimestamp(Arguments args) {
    if (!std::holds_alternative<String>(args[0])) {
        throw FunctionException("Hàm 'getFileTimestamp' yêu cầu tham số là chuỗi đường dẫn.");
    }
    const std::string& path_str = std::get<String>(args[0])->str;

    try {

        auto last_write_time = fs::last_write_time(path_str);
        

        auto epoch_time = last_write_time.time_since_epoch();
        

        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(epoch_time);

        return Value(static_cast<Real>(milliseconds.count()));
    } catch(const fs::filesystem_error& e) {
        throw FunctionException("Lỗi khi lấy timestamp của file '" + path_str + "': " + e.what());
    }
}

Value getFileSize(Arguments args) {
    if (!std::holds_alternative<String>(args[0])) {
        throw FunctionException("Hàm 'getFileSize' yêu cầu tham số là chuỗi đường dẫn.");
    }
    const std::string& path_str = std::get<String>(args[0])->str;
    try {
        return Value(static_cast<Real>(fs::file_size(path_str)));
    } catch(const fs::filesystem_error& e) {
        throw FunctionException("Lỗi khi lấy kích thước file '" + path_str + "': " + e.what());
    }
}

Value renameFile(Arguments args) {
    if (!std::holds_alternative<String>(args[0]) || !std::holds_alternative<String>(args[1])) {
        throw FunctionException("Hàm 'renameFile' yêu cầu hai tham số là chuỗi đường dẫn.");
    }
    const std::string& old_path = std::get<String>(args[0])->str;
    const std::string& new_path = std::get<String>(args[1])->str;
    try {
        fs::rename(old_path, new_path);
        return Value(Null{});
    } catch(const fs::filesystem_error& e) {
        throw FunctionException("Lỗi khi đổi tên file '" + old_path + "': " + e.what());
    }
}

Value copyFile(Arguments args) {
    if (!std::holds_alternative<String>(args[0]) || !std::holds_alternative<String>(args[1])) {
        throw FunctionException("Hàm 'copyFile' yêu cầu hai tham số là chuỗi đường dẫn.");
    }
    const std::string& source = std::get<String>(args[0])->str;
    const std::string& dest = std::get<String>(args[1])->str;
    try {

        fs::copy(source, dest, fs::copy_options::overwrite_existing);
        return Value(Null{});
    } catch(const fs::filesystem_error& e) {
        throw FunctionException("Lỗi khi sao chép file từ '" + source + "' tới '" + dest + "': " + e.what());
    }
}

Value getFileName(Arguments args) {
    if (!std::holds_alternative<String>(args[0])) {
        throw FunctionException("Hàm 'getFileName' yêu cầu tham số là chuỗi đường dẫn.");
    }
    const std::string& path_str = std::get<String>(args[0])->str;
    try {
        fs::path p = path_str;
        return Value(p.filename().string());
    } catch(const fs::filesystem_error& e) {
        throw FunctionException("Lỗi khi lấy tên file từ '" + path_str + "': " + e.what());
    }
}

Value getFileStem(Arguments args) {
    if (!std::holds_alternative<String>(args[0])) {
        throw FunctionException("Hàm 'getFileStem' yêu cầu tham số là chuỗi đường dẫn.");
    }
    const std::string& path_str = std::get<String>(args[0])->str;
    try {
        fs::path p = path_str;
        return Value(p.stem().string());
    } catch(const fs::filesystem_error& e) {
        throw FunctionException("Lỗi khi lấy tên file (stem) từ '" + path_str + "': " + e.what());
    }
}

Value getFileExtension(Arguments args) {
    if (!std::holds_alternative<String>(args[0])) {
        throw FunctionException("Hàm 'getFileExtension' yêu cầu tham số là chuỗi đường dẫn.");
    }
    const std::string& path_str = std::get<String>(args[0])->str;
    try {
        fs::path p = path_str;
        return Value(p.extension().string());
    } catch(const fs::filesystem_error& e) {
        throw FunctionException("Lỗi khi lấy đuôi file (extension) từ '" + path_str + "': " + e.what());
    }
}

Value getAbsolutePath(Arguments args) {
    if (!std::holds_alternative<String>(args[0])) {
        throw FunctionException("Hàm 'getAbsolutePath' yêu cầu tham số là chuỗi đường dẫn.");
    }
    const std::string& path_str = std::get<String>(args[0])->str;
    try {

        fs::path p = fs::absolute(path_str);
        return Value(p.string());
    } catch(const fs::filesystem_error& e) {
        throw FunctionException("Lỗi khi lấy đường dẫn tuyệt đối từ '" + path_str + "': " + e.what());
    }
}

IoLib::IoLib() {
    registerFn("input", input, Arity::range(0, 1));
    registerFn("read", readFile, 1);
    registerFn("write", writeFile, Arity::range(2, 3));
    registerFn("fileExists", fileExists, 1);
    registerFn("isDirectory", isDirectory, 1);
    registerFn("listDir", listDir, 1);
    registerFn("createDir", createDir, 1);
    registerFn("deleteFile", deleteFile, 1);

    registerFn("getFileTimestamp", getFileTimestamp, 1);
    registerFn("getFileSize", getFileSize, 1);
    registerFn("renameFile", renameFile, 2);
    registerFn("copyFile", copyFile, 2);

    registerFn("getFileName", getFileName, 1);
    registerFn("getFileStem", getFileStem, 1);
    registerFn("getFileExtension", getFileExtension, 1);
    registerFn("getAbsolutePath", getAbsolutePath, 1);
}