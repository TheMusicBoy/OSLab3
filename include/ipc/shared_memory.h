#pragma once

#include <common/exception.h>

#include <string>
#include <cstddef>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace NIpc {

////////////////////////////////////////////////////////////////////////////////

class TSharedMemory {
public:
    TSharedMemory(const std::string& name, size_t size, bool create = true);

    ~TSharedMemory();

    TSharedMemory(const TSharedMemory&) = delete;
    TSharedMemory& operator=(const TSharedMemory&) = delete;

    void* GetData();

    const void* GetData() const;

    size_t GetSize() const;

    const std::string& GetName() const;

    void Write(const void* data, size_t size, size_t offset = 0);

    void Read(void* buffer, size_t size, size_t offset = 0) const;

    void Close();

    static void Unlink(const std::string& name);

private:
    std::string name;
    size_t size;
#ifdef _WIN32
    HANDLE hMapFile;
#else
    int fd;
#endif
    void* data;
    bool isOpen;
};

////////////////////////////////////////////////////////////////////////////////

class TSharedMemoryException : public NCommon::TException {
public:
    template<typename... Args>
    explicit TSharedMemoryException(const std::string& format, Args&&... args)
        : NCommon::TException(format, std::forward<Args>(args)...) {}

    explicit TSharedMemoryException(const std::string& msg)
        : NCommon::TException(msg) {}
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NIpc
