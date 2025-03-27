#include <ipc/subprocess.h>

#include <common/logging.h>

#include <filesystem>
#include <service.h>
#include <signal.h>

namespace {

////////////////////////////////////////////////////////////////////////////////

std::ostream& DateTime(std::ostream& out) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm{};
    
#ifdef _WIN32
    localtime_s(&now_tm, &now_time);
#else
    localtime_r(&now_time, &now_tm);
#endif

    return out << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
}

////////////////////////////////////////////////////////////////////////////////

} // namespace

////////////////////////////////////////////////////////////////////////////////

bool IsAlive(NIpc::TProcessHandle pid) {
#ifdef _WIN32
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!process) return false;
    DWORD exitCode;
    GetExitCodeProcess(process, &exitCode);
    CloseHandle(process);
    return exitCode == STILL_ACTIVE;
#else
    return kill(pid, 0) == 0 || errno != ESRCH;
#endif
}

TService::TService(const std::string& path)
    : ThreadPool_(NCommon::New<NCommon::TThreadPool>(4)),
      Invoker_(NCommon::New<NCommon::TInvoker>(ThreadPool_)),
      Path_(std::filesystem::absolute(path).string())
{
    InitSharedMemory();
}

TService::~TService() {
    if (Storage_->MainProcessLock.try_lock()) {
        Storage_->MainProcessLock.unlock();
    }
}

void TService::InitSharedMemory() {
#ifdef _WIN32
    const std::string shmName = "Global\\service_storage";
#else
    const std::string shmName = "/service_storage"; 
#endif
    try {
        SharedMemory_ = std::make_unique<NIpc::TSharedMemory>(
            shmName, sizeof(TStorage), /*create*/ false);
        Storage_ = static_cast<TStorage*>(SharedMemory_->GetData());
        return;
    } catch (const NIpc::TSharedMemoryException& ex) {
        LOG_INFO("Shared memory is not created");
        // RETHROW(ex, "Shared memory initialization failed");
    }

    try {
        SharedMemory_ = std::make_unique<NIpc::TSharedMemory>(
            shmName, sizeof(TStorage), /*create*/ true);
        Storage_ = static_cast<TStorage*>(SharedMemory_->GetData());
        Storage_->Counter.store(0);
        Storage_->LoggingProcessLock.pid_.store(0);
    } catch (const NIpc::TSharedMemoryException& ex) {
        RETHROW(ex, "Shared memory initialization failed");
    }
}

void TService::Start() {
    LogWithLock("Started process with pid: {}", NIpc::GetPid());

    IncrementPeriodicExecutor_ = New<NCommon::TPeriodicExecutor>(
        NCommon::Bind(&TService::IncrementCounter, this),
        Invoker_,
        std::chrono::milliseconds(300)
    );
    IncrementPeriodicExecutor_->Start();
    
    PrintPeriodicExecutor_ = New<NCommon::TPeriodicExecutor>(
        NCommon::Bind(&TService::PrintLog, this),
        Invoker_,
        std::chrono::seconds(1)
    );
    PrintPeriodicExecutor_->Start();
    
    CreatePeriodicExecutor_ = New<NCommon::TPeriodicExecutor>(
        NCommon::Bind(&TService::CreateCopies, this),
        Invoker_,
        std::chrono::seconds(3)
    );
    CreatePeriodicExecutor_->Start();
}

bool TService::IsMain() {
    return Storage_->MainProcessLock.try_lock();
}

void TService::SetValue(int value) {
    Storage_->Counter.store(value, std::memory_order_relaxed);
}

void TService::IncrementCounter() {
    Storage_->Counter.fetch_add(1, std::memory_order_relaxed);
}

void TService::PrintLog() {
    if (!IsMain()) {
        return;
    }
    LogWithLock("Time: {}, Pid: {}, Counter value: {}",
        &DateTime, NIpc::GetPid(), Storage_->Counter.load(std::memory_order_relaxed));
}

void TService::CreateCopies() {
    if (!IsMain()) {
        return;
    }

    if (NIpc::IsProcessAlive(Storage_->RoleA_pid) || NIpc::IsProcessAlive(Storage_->RoleB_pid)) {
        return;
    }

    Storage_->RoleA_pid = NIpc::CreateSubprocess(
        NCommon::Format("{} --role-a", Path_)
    );

    Storage_->RoleB_pid = NIpc::CreateSubprocess(
        NCommon::Format("{} --role-b", Path_)
    );
}

void TService::RoleA() {
    LogWithLock("Time: {}, Pid: {}, RoleA started", &DateTime, NIpc::GetPid());
    Storage_->Counter.fetch_add(10, std::memory_order_acq_rel);
    LogWithLock("Time: {}, Pid: {}, RoleA finished", &DateTime, NIpc::GetPid());
}

void TService::RoleB() {
    LogWithLock("Time: {}, Pid: {}, RoleB started", &DateTime, NIpc::GetPid());
    for (int val = 0; !Storage_->Counter.compare_exchange_strong(val, val * 2, std::memory_order_acq_rel););
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    for (int val = 0; !Storage_->Counter.compare_exchange_strong(val, val / 2, std::memory_order_acq_rel););
    LogWithLock("Time: {}, Pid: {}, RoleB finished", &DateTime, NIpc::GetPid());
}
