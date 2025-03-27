#pragma once

#include <common/intrusive_ptr.h>
#include <common/refcounted.h>

#include <atomic>
#include <chrono>
#include <functional>

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

class TInvoker;

class TPeriodicExecutor : public NRefCounted::TRefCountedBase {
public:
    TPeriodicExecutor(
        std::function<bool()> callback,
        TIntrusivePtr<TInvoker> invoker,
        std::chrono::milliseconds delay
    );

    void Start();
    void Stop();

private:
    void ScheduleNext();

    void Worker();

    std::function<bool()> Callback_;
    TIntrusivePtr<TInvoker> Invoker_;
    std::chrono::milliseconds Delay_;
    std::atomic<bool> StopFlag_{false};
};

DECLARE_REFCOUNTED(TPeriodicExecutor);

////////////////////////////////////////////////////////////////////////////////

template<typename T>
struct IsWeakPtr : std::false_type {};

template<typename T>
struct IsWeakPtr<TWeakPtr<T>> : std::true_type {};

struct TExpiredWeakPtr {};

template<typename T>
auto ConvertWeakPtr(const T& obj) {
    if constexpr (IsWeakPtr<T>::value) {
        if (auto locked = obj.Lock()) {
            return locked;
        } else {
            throw TExpiredWeakPtr{};
        }
    } else {
        return obj;
    }
}

template <typename Callable, typename... Args>
auto Bind(Callable&& cb, Args&&... args) {
    return [
        callable = std::forward<Callable>(cb),
        bound_args = std::make_tuple(std::forward<Args>(args)...)
    ]() -> bool {
        try {
            std::apply([&](auto&&... items) {
                return std::invoke(callable, ConvertWeakPtr(items)...);
            }, bound_args);
            return false;
        } catch (TExpiredWeakPtr) {
            return true;
        } catch (std::exception& ex) {
            throw;
        }
    };
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon
