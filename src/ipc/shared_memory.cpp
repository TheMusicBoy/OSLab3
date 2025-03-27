#include <ipc/shared_memory.h>

#include <common/format.h>
#include <common/logging.h>

#include <cerrno>
#include <cstring>

namespace NIpc {

////////////////////////////////////////////////////////////////////////////////

TSharedMemory::TSharedMemory(const std::string& name, size_t size, bool create)
    : name(name), size(size), fd(-1), data(nullptr), isOpen(false) {
    
    if (name.empty() || name[0] != '/') {
        throw TSharedMemoryException("Shared memory name must start with '/', got: {}", name);
    }

#ifdef _WIN32
    std::string sanitizedName = name.substr(1); // Remove leading '/'
    std::replace(sanitizedName.begin(), sanitizedName.end(), '/', '\\');
    
    if (create) {
        hMapFile = CreateFileMappingA(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            sizeHigh,
            sizeLow,
            sanitizedName.c_str()
        );
    } else {
        hMapFile = OpenFileMappingA(
            FILE_MAP_ALL_ACCESS,
            FALSE,
            sanitizedName.c_str()
        );
    }

    if (hMapFile == NULL) {
        throw TSharedMemoryException("Failed to {} shared memory '{}': {}",
                                    create ? "create" : "open",
                                    sanitizedName,
                                    GetLastError());
    }

    data = MapViewOfFile(hMapFile,
                        FILE_MAP_ALL_ACCESS,
                        0,
                        0,
                        size);
    if (data == NULL) {
        CloseHandle(hMapFile);
        throw TSharedMemoryException("Failed to map shared memory '{}': {}",
                                    sanitizedName,
                                    GetLastError());
    }
#else
    if (name.empty() || name[0] != '/') {
        throw TSharedMemoryException("Shared memory name must start with '/', got: {}", name);
    }

    int flags = O_RDWR;
    if (create) {
        flags |= O_CREAT;
    }

    fd = shm_open(name.c_str(), flags, 0666);
    if (fd == -1) {
        throw TSharedMemoryException("Failed to open shared memory '{}': {}", 
                                   name, strerror(errno));
    }

    if (create && ftruncate(fd, size) == -1) {
        close(fd);
        throw TSharedMemoryException("Failed to set size of shared memory '{}': {}", 
                                   name, strerror(errno));
    }

    data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        close(fd);
        throw TSharedMemoryException("Failed to map shared memory '{}': {}", 
                                   name, strerror(errno));
    }
#endif

    isOpen = true;
    LOG_DEBUG("Shared memory '{}' opened, size: {} bytes", name, size);
}

TSharedMemory::~TSharedMemory() {
    Close();
}

void* TSharedMemory::GetData() {
    if (!isOpen) {
        throw TSharedMemoryException("Shared memory '{}' is not open", name);
    }
    return data;
}

const void* TSharedMemory::GetData() const {
    if (!isOpen) {
        throw TSharedMemoryException("Shared memory '{}' is not open", name);
    }
    return data;
}

size_t TSharedMemory::GetSize() const {
    return size;
}

const std::string& TSharedMemory::GetName() const {
    return name;
}

void TSharedMemory::Write(const void* sourceData, size_t writeSize, size_t offset) {
    if (!isOpen) {
        throw TSharedMemoryException("Shared memory '{}' is not open", name);
    }
    
    if (offset + writeSize > size) {
        throw TSharedMemoryException("Write operation would exceed shared memory bounds: "
                                    "offset({}) + size({}) > total size({})",
                                    offset, writeSize, size);
    }
    
    std::memcpy(static_cast<char*>(data) + offset, sourceData, writeSize);
}

void TSharedMemory::Read(void* buffer, size_t readSize, size_t offset) const {
    if (!isOpen) {
        throw TSharedMemoryException("Shared memory '{}' is not open", name);
    }
    
    if (offset + readSize > size) {
        throw TSharedMemoryException("Read operation would exceed shared memory bounds: "
                                    "offset({}) + size({}) > total size({})",
                                    offset, readSize, size);
    }
    
    std::memcpy(buffer, static_cast<const char*>(data) + offset, readSize);
}

void TSharedMemory::Close() {
    if (isOpen) {
#ifdef _WIN32
        if (data) {
            UnmapViewOfFile(data);
        }
        if (hMapFile) {
            CloseHandle(hMapFile);
        }
#else
        if (data != MAP_FAILED) {
            munmap(data, size);
        }
        if (fd != -1) {
            close(fd);
        }
#endif
        
        isOpen = false;
        LOG_DEBUG("Shared memory '{}' closed", name);
    }
}

void TSharedMemory::Unlink(const std::string& name) {
#ifdef _WIN32
    // Windows automatically cleans up when all handles are closed
#else
    if (shm_unlink(name.c_str()) == -1) {
        throw TSharedMemoryException("Failed to unlink shared memory '{}': {}", 
                                   name, strerror(errno));
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NIpc
