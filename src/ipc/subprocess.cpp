#include <ipc/subprocess.h>

#include <common/exception.h>
#include <common/logging.h>

#include <string.h>
#include <vector>

// #define _WIN32

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

namespace NIpc {

////////////////////////////////////////////////////////////////////////////////

TProcessHandle CreateSubprocess(std::string cmd) {
    ASSERT(!cmd.empty(), "Error: Empty command provided");

#if defined(_WIN32) || defined(_WIN64)
    std::unique_ptr<STARTUPINFOA> si = std::make_unique<STARTUPINFOA>();
    std::unique_ptr<PROCESS_INFORMATION> pi = std::make_unique<PROCESS_INFORMATION>();

    ZeroMemory(si.get(), sizeof(STARTUPINFOA));
    si->cb = sizeof(STARTUPINFOA);
    ZeroMemory(pi.get(), sizeof(PROCESS_INFORMATION));

    // Create a modifiable copy of the command
    cmd.push_back('\0'); // Ensure null termination

    if (!CreateProcessA(
            nullptr,
            cmd.data(),
            nullptr,
            nullptr,
            FALSE,
            0,
            nullptr,
            nullptr,
            si.get(),
            pi.get()
    )) {
        THROW("CreateProcess failed: {}", GetLastError());
    }

    DWORD pid = pi->dwProcessId;
    CloseHandle(pi->hThread);
    CloseHandle(pi->hProcess);
    return pid;
#else
    __pid_t pid = fork();
    
    if (pid == -1) {
        perror("failed fork process");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        char* text = cmd.data();

        std::vector<char*> args;
        while (char* token = strtok_r(text, " ", &text)) {
            args.push_back(token);
        }
        args.push_back(nullptr);

        execvp(args[0], args.data());

        NCommon::GetLogger().SetOutput("subprocess.log");
        LOG_ERROR("CreateProcess failed: {}", Errno);
        exit(EXIT_FAILURE);
    }

    return pid;
#endif
}

bool IsProcessAlive(TProcessHandle pid) {
    if (pid == 0) {
        return false;
    }
#ifdef _WIN32
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!process) return false;
    DWORD exitCode;
    GetExitCodeProcess(process, &exitCode);
    CloseHandle(process);
    return exitCode == STILL_ACTIVE;
#else
    int status;
    waitpid(pid, &status, WNOHANG | WEXITED);
    return status == 0;
#endif
}

TProcessHandle GetPid() {
#ifdef _WIN32
    return GetCurrentProcessId();
#else
    return getpid();
#endif
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NIpc
