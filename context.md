## Project Overview

This C++ project implements inter-process communication (IPC) mechanisms using POSIX shared memory and semaphores. The project demonstrates proper resource handling, synchronization, and error management for a shared counter application that can be accessed by multiple processes simultaneously.

## Project Structure

- `include/common/`/`src/common/` - Core utilities like logging, exception handling, and string formatting
- `include/ipc/`/`src/ipc` - Inter-process communication mechanisms (shared memory and semaphores)
- `src/main.cpp` - Main application implementing a shared counter

## Common Library Components

### Format System (`format.h`)
```cpp
// String formatting utility similar to Python's format
template<typename... Args>
std::string Format(const std::string& format, Args&&... args);
```

### Exception Handling (`exception.h`)
```cpp
class TException : public std::exception {
public:
    explicit TException(const std::string& msg);
    
    template<typename... Args>
    explicit TException(const std::string& format, Args&&... args);
    
    // Constructors with source location
    template<typename... Args>
    TException(const std::source_location& location, const std::string& format, Args&&... args);
    
    template<typename... Args>
    TException(const std::exception& e, const std::string& format, Args&&... args);
    
    template<typename... Args>
    TException(const std::source_location& location, const std::exception& e, 
              const std::string& format, Args&&... args);
              
    const char* what() const noexcept override;
};

// Helper functions and macros
template<typename... Args>
[[noreturn]] void ThrowException(const std::string& format, Args&&... args);

template<typename... Args>
[[noreturn]] void ThrowExceptionWithLocation(
    const std::source_location& location, 
    const std::string& format, 
    Args&&... args);

template<typename... Args>
[[noreturn]] void RethrowException(
    const std::exception& e,
    const std::string& format, 
    Args&&... args);

template<typename... Args>
[[noreturn]] void RethrowExceptionWithLocation(
    const std::source_location& location,
    const std::exception& e,
    const std::string& format, 
    Args&&... args);

// Macros
#define THROW(format, ...) // Throws exception with location
#define RETHROW(e, format, ...) // Rethrows with context and location
#define ASSERT(condition, format, ...) // Assertion check with failure message
```

### Logging System (`logging.h`)
```cpp
enum class ELogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

class TLogger {
public:
    void SetLevel(ELogLevel level);
    bool SetOutput(const std::string& filePath);
    void SetOutputToStdout();
    void SetOutputToStderr();
    void CloseOutput();
    
    // Logging methods for different levels
    template<typename... Args>
    void Debug(const std::string& format, Args&&... args);
    
    template<typename... Args>
    void Info(const std::string& format, Args&&... args);
    
    template<typename... Args>
    void Warning(const std::string& format, Args&&... args);
    
    template<typename... Args>
    void Error(const std::string& format, Args&&... args);
    
    template<typename... Args>
    void Fatal(const std::string& format, Args&&... args);
    
private:
    // Implementation details
};

// Global logger accessor
TLogger& GetLogger();

// Convenience macros
#define LOG_DEBUG(format, ...) 
#define LOG_INFO(format, ...)
#define LOG_WARNING(format, ...) 
#define LOG_ERROR(format, ...) 
#define LOG_FATAL(format, ...)
```

## IPC Library Components

### Shared Memory (`shared_memory.h`)
```cpp
class TSharedMemory {
public:
    // Constructor for creating or opening shared memory
    TSharedMemory(const std::string& name, size_t size, bool create = true, mode_t mode = 0666);
    ~TSharedMemory();
    
    // Non-copyable
    TSharedMemory(const TSharedMemory&) = delete;
    TSharedMemory& operator=(const TSharedMemory&) = delete;
    
    // Core methods
    void* GetData();
    const void* GetData() const;
    size_t GetSize() const;
    const std::string& GetName() const;
    
    // Data operations
    void Write(const void* data, size_t size, size_t offset = 0);
    void Read(void* buffer, size_t size, size_t offset = 0) const;
    
    // Resource management
    void Close();
    static void Unlink(const std::string& name);
    
private:
    std::string name;
    size_t size;
    int fd;
    void* data;
    bool isOpen;
};

// Exception class for shared memory errors
class TSharedMemoryException : public NCommon::TException {
    // Implementation inherits from TException
};
```

### Shared Semaphore (`shared_semaphore.h`)
```cpp
class TSharedSemaphore {
public:
    // Constructor for creating or opening semaphore
    TSharedSemaphore(const std::string& name, unsigned int initialValue, bool create = true, mode_t mode = 0666);
    ~TSharedSemaphore();
    
    // Core operations
    const std::string& GetName() const;
    void Wait();
    bool TryWait();
    bool TimedWait(const std::chrono::milliseconds& timeout);
    void Post();
    int GetValue() const;
    
    // Resource management
    void Close();
    static void Unlink(const std::string& name);
    
private:
    std::string name;
    sem_t* sem;
    bool isOpen;
};

// Exception class for semaphore errors
class TSharedSemaphoreException : public NCommon::TException {
    // Implementation inherits from TException
};
```

## Main Application

The main application implements a shared counter that can be accessed by multiple processes:

### SharedCounter Structure
```cpp
struct SharedCounter {
    int value;                // The counter value
    int updates;              // Number of updates made
    int activeProcesses;      // Number of processes currently using this shared memory
    char lastProcess[32];     // Name of the process that last updated
};
```

### Key Features
- Thread-safe access to the shared counter using semaphores
- Multiple processes can join and modify the counter
- Robust cleanup mechanism to ensure resources are released
- Signal handling for graceful termination
- Automatic resource cleanup when the last process exits

## Build System

### CMake Structure
- Main CMakeLists.txt with subdirectories
- Separate libraries for common and IPC components
- Links against system libraries (rt for POSIX shared memory)

```cmake
# Common library
add_library(common STATIC ${SRC})
target_include_directories(common PUBLIC ${PROJECT_SOURCE_DIR}/include)

# IPC library
add_library(ipc STATIC ${SRC})
target_include_directories(ipc PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(ipc PUBLIC common rt)

# Main executable
add_executable(main ${SRC})
target_include_directories(main PUBLIC ${INCROOT})
target_link_libraries(main PUBLIC common ipc)
```

## Usage Patterns

### Shared Memory Example
```cpp
// Create shared memory
NIpc::TSharedMemory shm("/my_shared_data", sizeof(MyData), true);

// Write data
MyData data{...};
shm.Write(&data, sizeof(data));

// Read data
MyData readData;
shm.Read(&readData, sizeof(readData));

// Clean up
shm.Close();
NIpc::TSharedMemory::Unlink("/my_shared_data");
```

### Semaphore Example
```cpp
// Create semaphore (initial value 1 = mutex)
NIpc::TSharedSemaphore sem("/my_semaphore", 1, true);

// Enter critical section
sem.Wait();

// ... access shared resources ...

// Exit critical section
sem.Post();

// Clean up
sem.Close();
NIpc::TSharedSemaphore::Unlink("/my_semaphore");
```

### Error Handling Pattern
```cpp
try {
    // IPC operations
    shm.Write(&data, sizeof(data));
} catch (const NIpc::TSharedMemoryException& e) {
    LOG_ERROR("Shared memory error: {}", e.what());
} catch (const std::exception& e) {
    LOG_ERROR("Generic error: {}", e.what());
}
```
