#pragma once

#include <common/logging.h>

#include <cstdlib>
#include <new>
#include <atomic>

namespace NRefCounted {

////////////////////////////////////////////////////////////////////////////////

class TRefCounter {
public:
    void Ref(int n = 1) const noexcept;

    bool TryRef() const noexcept;

    bool Unref(int n = 1) const;

    void WeakRef() const noexcept;

    bool WeakUnref() const;

private:
    mutable std::atomic<int> StrongRefCount_ = 1;
    mutable std::atomic<int> WeakRefCount_ = 1;
};

class TRefCountedBase {
public:
    TRefCountedBase() = default;

    TRefCountedBase(const TRefCountedBase&) = delete;
    TRefCountedBase(TRefCountedBase&&) = delete;

    TRefCountedBase& operator=(const TRefCountedBase&) = delete;
    TRefCountedBase& operator=(TRefCountedBase&&) = delete;
};

////////////////////////////////////////////////////////////////////////////////

// Forward declaration.
template <class T>
class TIntrusivePtr;

#define DECLARE_REFCOUNTED(type) \
    using type ## Ptr = ::NCommon::TIntrusivePtr<type>;

////////////////////////////////////////////////////////////////////////////////

template <typename T>
class TRefCountedHelper {
private:
    static constexpr size_t Align_ = std::max(alignof(TRefCounter), alignof(T));
    static constexpr size_t RefCounterSize_ = sizeof(TRefCounter);
    static constexpr size_t RefCounterOffset_ = (RefCounterSize_ + Align_ - 1) / Align_ * Align_;
    static constexpr size_t TotalAllocSize_ = RefCounterOffset_ + sizeof(T);

public:
    static T* Allocate() {
        const size_t aligned_size = (TotalAllocSize_ + Align_ - 1) / Align_ * Align_;
        void* ptr = std::aligned_alloc(Align_, aligned_size);
        if (!ptr) {
            throw std::bad_alloc();
        }
        
        new (ptr) TRefCounter();
        T* objectPtr = reinterpret_cast<T*>(static_cast<char*>(ptr) + RefCounterOffset_);
        return objectPtr;
    }

    static void Deallocate(void* ptr) {
        std::free(static_cast<char*>(ptr) - RefCounterOffset_);
    }

    template <typename... Args>
    static void Construct(void* ptr, Args&&... args) {
        new (ptr) T(std::forward<Args>(args)...);
    }

    static void Destruct(void* ptr) {
        static_cast<T*>(ptr)->~T();

        if (GetRefCounter(ptr)->WeakUnref()) {
            Deallocate(ptr);
        }
    }

    static TRefCounter* GetRefCounter(void* ptr) {
        return reinterpret_cast<TRefCounter*>(static_cast<char*>(ptr) - RefCounterOffset_);
    }
};

////////////////////////////////////////////////////////////////////////////////

template <class T>
inline void Ref(T* obj, int n = 1) {
    TRefCountedHelper<T>::GetRefCounter(obj)->Ref(n);
}

template <class T>
inline bool TryRef(T* obj) {
    return TRefCountedHelper<T>::GetRefCounter(obj)->TryRef();
}

template <class T>
inline void Unref(T* obj, int n = 1) {
    if (TRefCountedHelper<T>::GetRefCounter(obj)->Unref(n)) {
        TRefCountedHelper<T>::Destruct(obj);
    }
}

template <class T>
inline void WeakRef(T* obj) {
    TRefCountedHelper<T>::GetRefCounter(obj)->WeakRef();
}

template <class T>
inline void WeakUnref(T* obj) {
    if (TRefCountedHelper<T>::GetRefCounter(obj)->WeakUnref()) {
        TRefCountedHelper<T>::Deallocate(obj);
    }
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NRefCounted
