#pragma once

#include <common/intrusive_ptr.h>
#include <common/public.h>
#include <common/refcounted.h>

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

template <typename T>
class TWeakPtr {
public:
    TWeakPtr() : ptr_(nullptr) {}

    explicit TWeakPtr(void* ptr)
        : ptr_(static_cast<T*>(ptr))
    {
        if (ptr_) {
            NRefCounted::WeakRef(ptr_);
        }
    }

    TWeakPtr(const TWeakPtr& other)
        : ptr_(other.ptr_)
    {
        if (ptr_) {
            WeakRef(ptr_);
        }
    }
    
    TWeakPtr(TWeakPtr&& other) noexcept
        : ptr_(other.ptr_)
    {
        other.ptr_ = nullptr;
    }

    TWeakPtr(const TIntrusivePtr<T>& other)
        : ptr_(other.ptr_)
    {
        if (ptr_) {
            WeakRef(ptr_);
        }
    }

    ~TWeakPtr() {
        if (ptr_) {
            NRefCounted::WeakUnref(ptr_);
        }
    }

    TWeakPtr& operator=(const TWeakPtr& other) {
        if (this != &other) {
            ptr_ = other.ptr_;
        }
        return *this;
    }

    TIntrusivePtr<T> Lock() const {
        return ptr_ && NRefCounted::TryRef(ptr_)
            ? TIntrusivePtr<T>(ptr_, false)
            : TIntrusivePtr<T>();
    }

private:
    T* ptr_;
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon

////////////////////////////////////////////////////////////////////////////////

template <typename T>
NCommon::TWeakPtr<T> MakeWeak(T* ptr) {
    return NCommon::TWeakPtr<T>(ptr);
}
