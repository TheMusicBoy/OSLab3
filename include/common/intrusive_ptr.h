#pragma once

#include <common/refcounted.h>

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

template <typename T>
class TWeakPtr;

template <typename T>
class TIntrusivePtr;

template <typename T, typename... Args>
TIntrusivePtr<T> New(Args&&... args);

template <typename T>
class TIntrusivePtr {
public:
    explicit TIntrusivePtr() : ptr_(nullptr) {}

    explicit TIntrusivePtr(T* ptr)
        : ptr_(ptr)
    {
        if (ptr_) {
            NRefCounted::Ref(ptr_);
        }
    }

    TIntrusivePtr(const TIntrusivePtr& other)
        : ptr_(other.ptr_)
    {
        if (ptr_) {
            NRefCounted::Ref(ptr_);
        }
    }

    TIntrusivePtr(TIntrusivePtr&& other) noexcept
        : ptr_(other.ptr_)
    {
        other.ptr_ = nullptr;
    }

    TIntrusivePtr& operator=(const TIntrusivePtr& other) {
        if (this != &other) {
            reset();
            ptr_ = other.ptr_;
            if (ptr_) {
                NRefCounted::Ref(ptr_);
            }
        }
        return *this;
    }

    TIntrusivePtr& operator=(TIntrusivePtr&& other) noexcept {
        if (this != &other) {
            reset();
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }

    ~TIntrusivePtr() {
        reset();
    }

    T& operator*() const {
        return *ptr_;
    }

    T* operator->() const {
        return ptr_;
    }

    operator bool() const {
        return ptr_;
    }

    void reset() {
        if (ptr_) {
            NRefCounted::Unref(ptr_);
            ptr_ = nullptr;
        }
    }

private:
    T* ptr_;

    explicit TIntrusivePtr(T* object, bool bump = true)
        : ptr_(object)
    {
        if (ptr_ && bump) {
            NRefCounted::Ref(ptr_);
        }
    }

    template <typename U, typename... Args>
    friend TIntrusivePtr<U> New(Args&&... args);

    friend TWeakPtr<T>;
};

////////////////////////////////////////////////////////////////////////////////

template <typename T, typename... Args>
TIntrusivePtr<T> New(Args&&... args) {
    TIntrusivePtr<T> newPtr(NRefCounted::TRefCountedHelper<T>::Allocate(), false);
    try {
        NRefCounted::TRefCountedHelper<T>::Construct(newPtr.ptr_, std::forward<Args>(args)...);
    } catch (std::exception& ex) {
        NRefCounted::TRefCountedHelper<T>::Deallocate(newPtr.ptr_);
        newPtr.ptr_ = nullptr;
        throw;
    }
    return newPtr;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon  
