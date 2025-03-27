#include <common/exception.h>
#include <common/periodic_executor.h>
#include <common/threadpool.h>
#include <common/weak_ptr.h>

#include <thread>

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

TPeriodicExecutor::TPeriodicExecutor(
    std::function<bool()> callback,
    TIntrusivePtr<TInvoker> invoker,
    std::chrono::milliseconds delay
) : Callback_(std::move(callback)),
    Invoker_(std::move(invoker)),
    Delay_(delay)
{}

void TPeriodicExecutor::Start() {
    ScheduleNext();
}

void TPeriodicExecutor::Stop() {
    StopFlag_.store(true, std::memory_order_relaxed);
}

void TPeriodicExecutor::ScheduleNext() {
    if (StopFlag_.load(std::memory_order_relaxed)) return;

    Invoker_->Run(Bind(&TPeriodicExecutor::Worker, TWeakPtr<TPeriodicExecutor>(this)));
}

void TPeriodicExecutor::Worker() {
    if (StopFlag_.load(std::memory_order_relaxed)) return;

    bool shouldStop = false;
    try {
        shouldStop = Callback_();
    } catch (std::exception& ex) {
        RETHROW(ex, "PeriodicExecutor failed");
    }

    if (shouldStop) {
        StopFlag_.store(true);
        return;
    }

    std::this_thread::sleep_for(Delay_);
    ScheduleNext();
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon
