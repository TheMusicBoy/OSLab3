#pragma once

#include <cstring>
#include <sstream>
#include <string>

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

template <typename T>
inline void FormatHandler(std::ostringstream& out, const T& value) {
    out << value;
}

struct errno_type { int error_num; };
#define Errno ::NCommon::errno_type{errno}

inline void FormatHandler(std::ostringstream& out, const errno_type& value) {
    out << std::strerror(value.error_num);
}

////////////////////////////////////////////////////////////////////////////////

namespace detail {

////////////////////////////////////////////////////////////////////////////////

inline void FormatImpl(std::ostringstream& result, 
                       const std::string& format, 
                       size_t& pos) {
    result << format.substr(pos);
}

template<typename T, typename... Rest>
void FormatImpl(std::ostringstream& result, 
                const std::string& format, 
                size_t& pos, 
                T&& value, 
                Rest&&... rest) {
    size_t next = format.find("{}", pos);
    if (next == std::string::npos) {
        result << format.substr(pos);
        return;
    }
    
    result << format.substr(pos, next - pos);
    ::NCommon::FormatHandler(result, value);
    pos = next + 2;
    FormatImpl(result, format, pos, std::forward<Rest>(rest)...);
}

////////////////////////////////////////////////////////////////////////////////

} // namespace detail

////////////////////////////////////////////////////////////////////////////////

template<typename... Args>
std::string Format(const std::string& format, Args&&... args) {
    std::ostringstream result;
    size_t pos = 0;
    detail::FormatImpl(result, format, pos, std::forward<Args>(args)...);
    return result.str();
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon
