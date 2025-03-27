#pragma once

#include <ipc/shared_memory.h>
#include <ipc/subprocess.h>

#include <common/periodic_executor.h>
#include <common/threadpool.h>

#include <atomic>

bool IsAlive(NIpc::TProcessHandle pid);

class TService : public NRefCounted::TRefCountedBase {
private:
    struct TProcessMutex {
        std::atomic<NIpc::TProcessHandle> pid_;

        void lock() {
            int iter = 0;
            for (NIpc::TProcessHandle value = 0; !pid_.compare_exchange_strong(value, NIpc::GetPid(), std::memory_order_acquire); value = 0) {
                NIpc::TProcessHandle currentPid = pid_.load(std::memory_order_acquire);
                if (++iter % 1000 == 0 && !IsAlive(currentPid)) {
                    if (pid_.compare_exchange_strong(currentPid, NIpc::GetPid(), std::memory_order_acquire)) {
                        return;
                    }
                }
            }
        }

        bool try_lock() {
            NIpc::TProcessHandle currentPid = pid_.load(std::memory_order_acquire);
            if (currentPid == NIpc::GetPid()) {
                return true;
            }

            if (currentPid == 0) {
                return pid_.compare_exchange_strong(currentPid, NIpc::GetPid(), std::memory_order_acquire);
            }

            if (IsAlive(currentPid)) {
                return false;
            }

            return pid_.compare_exchange_strong(currentPid, NIpc::GetPid(), std::memory_order_acquire);
        }

        void unlock() {
            NIpc::TProcessHandle currentPid = NIpc::GetPid();
            VERIFY(pid_.compare_exchange_strong(currentPid, 0, std::memory_order_release));
        }
    };

    struct TStorage {
        std::atomic<int> Counter;
        TProcessMutex MainProcessLock;
        TProcessMutex LoggingProcessLock;
        NIpc::TProcessHandle RoleA_pid;
        NIpc::TProcessHandle RoleB_pid;
    };

    std::unique_ptr<NIpc::TSharedMemory> SharedMemory_;
    
    TStorage* Storage_;

    NCommon::TIntrusivePtr<NCommon::TThreadPool> ThreadPool_;
    NCommon::TIntrusivePtr<NCommon::TInvoker> Invoker_;
    NCommon::TPeriodicExecutorPtr IncrementPeriodicExecutor_;
    NCommon::TPeriodicExecutorPtr PrintPeriodicExecutor_;
    NCommon::TPeriodicExecutorPtr CreatePeriodicExecutor_;

    std::string Path_;

    template <typename... Args>
    void LogWithLock(Args&&... args) {
        auto guard = std::lock_guard(Storage_->LoggingProcessLock);
        std::ofstream logfile("service.log", std::ios::app);
        logfile << NCommon::Format(std::forward<Args>(args)...) << std::endl;
    }

    void InitSharedMemory();

public:
    TService(const std::string& path);
    ~TService();

    void Start();
    bool IsMain();
    void SetValue(int value);
    void IncrementCounter();
    void PrintLog();
    void CreateCopies();
    void RoleA();
    void RoleB();
};

DECLARE_REFCOUNTED(TService);
