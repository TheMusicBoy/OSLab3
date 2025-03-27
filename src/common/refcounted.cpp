#include <common/refcounted.h>
#include <common/exception.h>

namespace NRefCounted {

////////////////////////////////////////////////////////////////////////////////

void TRefCounter::Ref(int n) const noexcept {
    VERIFY(n >= 0);

    int value = StrongRefCount_.fetch_add(n, std::memory_order_relaxed);
    VERIFY(value > 0);
    VERIFY(WeakRefCount_.load(std::memory_order_relaxed) > 0);
}

bool TRefCounter::TryRef() const noexcept {
    auto value = StrongRefCount_.load(std::memory_order_relaxed);
    VERIFY(value >= 0);
    VERIFY(WeakRefCount_.load(std::memory_order_relaxed) > 0);

    while (value != 0 && !StrongRefCount_.compare_exchange_weak(value, value + 1));
    return value != 0;
}

bool TRefCounter::Unref(int n) const {
    VERIFY(n >= 0);

    auto oldStrongCount = StrongRefCount_.fetch_sub(n, std::memory_order_release);
    VERIFY(oldStrongCount >= n);
    if (oldStrongCount == n) {
        std::atomic_thread_fence(std::memory_order::acquire);
        return true;
    } else {
        return false;
    }
}

void TRefCounter::WeakRef() const noexcept {
    auto oldWeakCount = WeakRefCount_.fetch_add(1, std::memory_order_relaxed);
    VERIFY(oldWeakCount > 0);
}

bool TRefCounter::WeakUnref() const {
    auto oldWeakCount = WeakRefCount_.fetch_sub(1, std::memory_order_release);
    VERIFY(oldWeakCount > 0);
    if (oldWeakCount == 1) {
        std::atomic_thread_fence(std::memory_order::acquire);
        return true;
    } else {
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NRefCounted
