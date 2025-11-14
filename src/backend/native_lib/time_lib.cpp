#include "native_lib/standard_lib.hpp"
#include "diagnostics/meow_exceptions.hpp"
#include <chrono>
#include <thread>

Value native_clock(Arguments args) {

    auto now = std::chrono::high_resolution_clock::now();

    auto duration = now.time_since_epoch();

    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(duration).count();
    return Value(Real(seconds));
}

Value native_sleep(Arguments args) {
    if (!std::holds_alternative<Int>(args[0]) && !std::holds_alternative<Real>(args[0])) {
        throw FunctionException("Hàm sleep() cần một tham số là số (giây).");
    }
    
    Real seconds = std::holds_alternative<Int>(args[0]) 
                   ? static_cast<Real>(std::get<Int>(args[0]))
                   : std::get<Real>(args[0]);

    if (seconds < 0) {
        throw FunctionException("Thời gian sleep không thể là số âm.");
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long long>(seconds * 1000)));
    
    return Value(Null{});
}

Value native_time_now(Arguments args) {
    auto now = std::chrono::system_clock::now();
    return Value((Int)std::chrono::system_clock::to_time_t(now));
}


Value native_time_format(Arguments args) {
    const auto& format_str = std::get<String>(args[0])->str;

    auto now_point = std::chrono::system_clock::now();
    if (args.size() > 1) {
        Int timestamp = std::get<Int>(args[1]);
        now_point = std::chrono::system_clock::from_time_t(timestamp);
    }
    
    std::time_t time_now_t = std::chrono::system_clock::to_time_t(now_point);
    std::tm tm_now = *std::localtime(&time_now_t);

    std::ostringstream ss;
    ss << std::put_time(&tm_now, format_str.c_str());
    
    return Value(ss.str());
}

TimeLib::TimeLib() {
    registerFn("clock", native_clock, 0);
    registerFn("sleep", native_sleep, 1);
    registerFn("now", native_time_now, 0);
    registerFn("format", native_time_format, Arity::range(1, 1));
}