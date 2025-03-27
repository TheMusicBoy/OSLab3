#include <common/logging.h>

namespace NCommon {

TLogger& GetLogger() {
    static TLogger instance;
    return instance;
}

TLogger::TLogger() 
    : currentLevel(ELogLevel::Info), output(&std::cout) {
}

void TLogger::SetLevel(ELogLevel level) {
    std::lock_guard<std::mutex> lock(mutex);
    currentLevel = level;
}

bool TLogger::SetOutput(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (fileOutput.is_open()) {
        fileOutput.close();
    }
    
    fileOutput.open(filePath, std::ios::out | std::ios::app);
    if (!fileOutput.is_open()) {
        output = &std::cout;
        return false;
    }
    
    output = &fileOutput;
    return true;
}
void TLogger::CloseOutput() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (fileOutput.is_open()) {
        fileOutput.close();
    }
    
    output = &std::cout;
}

void TLogger::SetOutputToStdout() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (fileOutput.is_open()) {
        fileOutput.close();
    }
    
    output = &std::cout;
}

void TLogger::SetOutputToStderr() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (fileOutput.is_open()) {
        fileOutput.close();
    }
    
    output = &std::cerr;
}

std::string TLogger::LevelToString(ELogLevel level) {
    switch (level) {
        case ELogLevel::Debug:   return "DEBUG";
        case ELogLevel::Info:    return "INFO";
        case ELogLevel::Warning: return "WARNING";
        case ELogLevel::Error:   return "ERROR";
        case ELogLevel::Fatal:   return "FATAL";
        default:                 return "UNKNOWN";
    }
}

} // namespace NCommon

