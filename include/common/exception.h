#pragma once

#include <common/format.h>

#include <exception>
#include <source_location>
#include <string>

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

class TException : public std::exception {
public:
    explicit TException(const std::string& msg) : message(msg) {}

    explicit TException(const std::exception& ex) : message(ex.what()) {}

    template<typename... Args>
    explicit TException(const std::string& format, Args&&... args)
        : message(Format(format, std::forward<Args>(args)...)) {}

    template<typename... Args>
    TException(const std::source_location& location, const std::string& format, Args&&... args)
        : message(Format("{}:{}: {}", location.file_name(), location.line(), 
                         Format(format, std::forward<Args>(args)...))) {}
    
    template<typename... Args>
    TException(const std::exception& e, const std::string& format, Args&&... args)
        : message(Format("{}: {}", Format(format, std::forward<Args>(args)...), e.what())) {}
    
    template<typename... Args>
    TException(const std::source_location& location, const std::exception& e, 
              const std::string& format, Args&&... args)
        : message(Format("{}:{}: {}:\n{}", 
                         location.file_name(), location.line(),
                         Format(format, std::forward<Args>(args)...), 
                         e.what())) {}

    const char* what() const noexcept override {
        return message.c_str();
    }

protected:
    std::string message;
};

////////////////////////////////////////////////////////////////////////////////

template<typename... Args>
[[noreturn]] void ThrowException(const std::string& format, Args&&... args) {
    throw TException(format, std::forward<Args>(args)...);
}

template<typename... Args>
[[noreturn]] void ThrowExceptionWithLocation(
    const std::source_location& location, 
    const std::string& format, 
    Args&&... args) {
    throw TException(location, format, std::forward<Args>(args)...);
}

template<typename... Args>
[[noreturn]] void RethrowException(
    const std::exception& e,
    const std::string& format, 
    Args&&... args) {
    throw TException(e, format, std::forward<Args>(args)...);
}

template<typename... Args>
[[noreturn]] void RethrowExceptionWithLocation(
    const std::source_location& location,
    const std::exception& e,
    const std::string& format, 
    Args&&... args) {
    throw TException(location, e, format, std::forward<Args>(args)...);
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon

////////////////////////////////////////////////////////////////////////////////

#define THROW(format, ...) \
    ::NCommon::ThrowExceptionWithLocation(\
        std::source_location::current(), format, ##__VA_ARGS__)

#define RETHROW(e, format, ...) \
    ::NCommon::RethrowExceptionWithLocation(\
        std::source_location::current(), e, format, ##__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////

#define ASSERT(condition, format, ...) \
    if (!(condition)) { \
        THROW("Assertion failed: " format, ##__VA_ARGS__); \
    }

#ifdef _WIN32
#define DEBUG_TRAP() __debugbreak()
#else
#define DEBUG_TRAP() __builtin_trap()
#endif

#define VERIFY(condition) if (!(condition)) { DEBUG_TRAP(); }
