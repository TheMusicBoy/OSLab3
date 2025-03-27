#include <ipc/subprocess.h>
#include <common/exception.h>

#include <vector>
#include <string.h>

// #define _WIN32

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>
#endif

namespace NSubprocess {

////////////////////////////////////////////////////////////////////////////////

TProcessHandle CreateSubprocess(std::string cmd) {
    if (cmd.empty()) {
        throw std::invalid_argument("Empty command provided");
    }

#ifdef _WIN32
    STARTUPINFO si{};
    PROCESS_INFORMATION pi;
    int success = CreateProcess(
        command.data(), // The path
        nullptr, // Command line
        nullptr, // Process handle not inheritable
        nullptr, // Thread handle not inheritable
        FALSE, // Set handle inheritance to FALSE
        0, // No creation flags
        nullptr, // Use parent's environment block
        nullptr, // Use parent's starting directory
        &si, // Pointer to STARTUPINFO structure
        &pi); // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
    

    if (success == 0) {
        status = GetLastError();
    }

    return pi.dwProcessId;
#else
    pid_t pid;

    char* data = cmd.data();
    std::vector<char*> args;
    while (auto token = strtok_r(data, " ", &data)) {
        args.push_back(token);
    }

    int status = posix_spawnp(
        &pid, // Pid output
        args[0], // File to execute
        nullptr, // No actions between fork and exec
        nullptr, // Standart attributes for process
        args.data(), // Arguments for process
        nullptr); // Use parent's environment

    return pid;
#endif
}

////////////////////////////////////////////////////////////////////////////////

bool IsProcessAlive(TProcessHandle pid) {
#ifdef _WIN32
    DWORD exitCode;
    if (!GetExitCodeProcess(pid, &exitCode))
        return false;
    return exitCode == STILL_ACTIVE;
#else
    return kill(pid, 0) == 0 || errno != ESRCH;
#endif
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NSubprocess
