#pragma once

#include <common/format.h>

#include <fstream>
#include <iostream>
#include <mutex>
#include <string>

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

enum class ELogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

////////////////////////////////////////////////////////////////////////////////

class TLogger;

TLogger& GetLogger();

class TLogger {
public:
    void SetLevel(ELogLevel level);
    
    bool SetOutput(const std::string& filePath);
    
    void SetOutputToStdout();
    
    void SetOutputToStderr();
    
    void CloseOutput();

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
    TLogger();
    
    friend TLogger& GetLogger();
    
    template<typename... Args>
    void Log(ELogLevel level, const std::string& format, Args&&... args);
    
    std::string LevelToString(ELogLevel level);
    
    ELogLevel currentLevel = ELogLevel::Info;
    
    std::ostream* output = &std::cout;
    
    std::ofstream fileOutput;
    
    std::mutex mutex;
};

////////////////////////////////////////////////////////////////////////////////

template<typename... Args>
void TLogger::Debug(const std::string& format, Args&&... args) {
    Log(ELogLevel::Debug, format, std::forward<Args>(args)...);
}

template<typename... Args>
void TLogger::Info(const std::string& format, Args&&... args) {
    Log(ELogLevel::Info, format, std::forward<Args>(args)...);
}

template<typename... Args>
void TLogger::Warning(const std::string& format, Args&&... args) {
    Log(ELogLevel::Warning, format, std::forward<Args>(args)...);
}

template<typename... Args>
void TLogger::Error(const std::string& format, Args&&... args) {
    Log(ELogLevel::Error, format, std::forward<Args>(args)...);
}

template<typename... Args>
void TLogger::Fatal(const std::string& format, Args&&... args) {
    Log(ELogLevel::Fatal, format, std::forward<Args>(args)...);
}

template<typename... Args>
void TLogger::Log(ELogLevel level, const std::string& format, Args&&... args) {
    if (level < currentLevel) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex);
    
    *output << "[" << LevelToString(level) << "] "
            << Format(format, std::forward<Args>(args)...) << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

#define LOG_DEBUG(format, ...) NCommon::GetLogger().Debug(format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) NCommon::GetLogger().Info(format, ##__VA_ARGS__)
#define LOG_WARNING(format, ...) NCommon::GetLogger().Warning(format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) NCommon::GetLogger().Error(format, ##__VA_ARGS__)
#define LOG_FATAL(format, ...) NCommon::GetLogger().Fatal(format, ##__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon
