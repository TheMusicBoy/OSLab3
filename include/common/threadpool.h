#pragma once

#include <common/exception.h>
#include <common/intrusive_ptr.h>

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <variant>
#include <vector>

namespace NCommon {

////////////////////////////////////////////////////////////////////////////////

class TThreadPool {
public:
    explicit TThreadPool(size_t numThreads);

    ~TThreadPool();

    template <typename F, typename... Args>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            tasks_.emplace(std::forward<F>(f));
        }
        cv_.notify_one();
    }

private:
    void Worker();

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable cv_;
    std::atomic<bool> stop_;
};

DECLARE_REFCOUNTED(TThreadPool);

////////////////////////////////////////////////////////////////////////////////

template <typename TError, typename Type>
class TErrorOrBase {
public:
    TErrorOrBase(Type value)
        : Value_(value), IsOkay_(true) {}

    TErrorOrBase(const TError& error)
        : Value_(error), IsOkay_(false) {}

    template <typename UError>
    TErrorOrBase(const UError& error)
        : Value_(TError(error)), IsOkay_(false) {}

    Type Value() const {
        return std::get<Type>(Value_);
    }

    Type ValueOrThrow() const {
        if (!IsOkay_) {
            throw std::get<TError>(Value_);
        }
        return std::get<Type>(Value_);
    }

    TError Error() const {
        if (IsOkay_) {
            throw std::runtime_error("No error present");
        }
        return std::get<Error>(Value_);
    }

    void ThrowOnError() const {
        if (!IsOkay_) {
            throw std::get<TError>(Value_);
        }
    }

    bool operator==(const TErrorOrBase& other) const {
        return Value_ == other.Value_;
    }

    bool operator!=(const TErrorOrBase& other) const {
        return !(*this == other);
    }

    explicit operator bool() const {
        return IsOkay_;
    }

private:
    std::variant<TError, Type> Value_;
    bool IsOkay_;
};

////////////////////////////////////////////////////////////////////////////////

template <typename TError>
class TErrorOrBase<TError, void> {
public:
    TErrorOrBase()
        : Value_({}), IsOkay_(true) {}

    TErrorOrBase(const TError& error)
        : Value_(error), IsOkay_(false) {}

    template <typename UError>
    TErrorOrBase(const UError& error)
        : Value_(TError(error)), IsOkay_(false) {}

    TError Error() const {
        return Value_.value();
    }

    void ThrowOnError() const {
        if (!IsOkay_) {
            throw std::get<TError>(Value_);
        }
    }

    bool operator==(const TErrorOrBase& other) const {
        return Value_ == other.Value_;
    }

    bool operator!=(const TErrorOrBase& other) const {
        return !(*this == other);
    }

    explicit operator bool() const {
        return IsOkay_;
    }

private:
    std::optional<TError> Value_;
    bool IsOkay_;
};

template <typename Type>
using TErrorOr = TErrorOrBase<::NCommon::TException, Type>;

////////////////////////////////////////////////////////////////////////////////

class TInvoker {
public:
    explicit TInvoker(TIntrusivePtr<TThreadPool> threadPool)
        : ThreadPool_(std::move(threadPool)) {}

    template <typename Callable, typename... Args>
    std::future<TErrorOr<std::invoke_result_t<Callable, Args...>>> Run(Callable&& callable, Args&&... args) {
        using ReturnType = std::invoke_result_t<Callable, Args...>;

        auto promise = std::make_shared<std::promise<TErrorOr<ReturnType>>>();
        auto future = promise->get_future();


        ThreadPool_->enqueue([callable = std::forward<Callable>(callable),
                     args = std::tuple(std::forward<Args>(args)...),
                     promise]() mutable {
            try {
                promise->set_value(TErrorOr<ReturnType>(
                    std::apply(callable, args)
                ));
            } catch (std::exception& ex) {
                promise->set_value(TErrorOr<ReturnType>(ex));
            }
        });

        return future;
    }

    template <typename Callable, typename... Args>
    requires(std::is_same_v<std::invoke_result_t<Callable, Args...>, void>)
    std::future<TErrorOr<void>> Run(Callable&& callable, Args&&... args) {
        auto promise = std::make_shared<std::promise<TErrorOr<void>>>();
        auto future = promise->get_future();


        ThreadPool_->enqueue([callable = std::forward<Callable>(callable),
                     args = std::tuple(std::forward<Args>(args)...),
                     promise]() mutable {
            try {
                std::apply(
                    std::forward<Callable>(callable),
                    std::move(args)
                );
                promise->set_value(TErrorOr<void>());
            } catch (std::exception& ex) {
                promise->set_value(TErrorOr<void>(ex));
            }
        });

        return future;
    }

private:
    TIntrusivePtr<TThreadPool> ThreadPool_;
};

DECLARE_REFCOUNTED(TInvoker);

////////////////////////////////////////////////////////////////////////////////

} // namespace NCommon
