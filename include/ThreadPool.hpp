#ifndef _HX_THREADPOOL_H_
#define _HX_THREADPOOL_H_ 1

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <tuple>
#include <type_traits>

#ifdef __HX_SUPPORT_TBB
#include <tbb/concurrent_queue.h>
#endif

#ifdef __HX_SUPPORT_BOOST
#include <boost/lockfree/queue.hpp>
#endif

namespace hx {
namespace __internal {
class __PoolTaskBase {
public:
    virtual void operator()(){};
    virtual ~__PoolTaskBase(){};
};

template <typename Function, typename... Args>
class __PromiseTask : public __PoolTaskBase {
    typedef std::invoke_result_t<std::decay_t<Function>, std::decay_t<Args>...> Result_t;

public:
    __PromiseTask(std::promise<Result_t> &&promise, Function &&f, Args &&... args)
        : _callFunction(std::move(f))
        , _callArguments(std::make_tuple(std::forward<Args>(args)...))
        , _callResult(std::move(promise)) {}
    ~__PromiseTask() override{};

    void operator()() noexcept override {
        try {
            _callResult.set_value(std::apply(_callFunction, _callArguments));
        } catch (...) {
            _callResult.set_exception(std::current_exception());
        }
    }

private:
    std::function<Result_t(Args...)> _callFunction;
    std::tuple<Args...> _callArguments;
    std::promise<Result_t> _callResult;
};

template <typename QueueType>
class __QueueAdapterStd {
public:
    typedef QueueType value_type;

    void push(QueueType item) {
        std::unique_lock lock(_taskQueueSync);
        _taskQueue.push(std::move(item));
    }

    bool pop(QueueType &result) {
        if (!_taskQueue.empty()) {
            std::unique_lock lock(_taskQueueSync);
            if (!_taskQueue.empty()) {
                result = std::move(_taskQueue.front());
                _taskQueue.pop();
                return true;
            }
        }

        return false;
    }

    template <typename IteratorBegin,
              typename IteratorEnd,
              typename = typename std::enable_if<std::is_convertible<
                  typename std::iterator_traits<IteratorBegin>::value_type,
                  QueueType>::value>::type>
    void push_many(IteratorBegin begin, IteratorEnd end) {
        std::unique_lock lock(_taskQueueSync);
        for (IteratorBegin it = begin; it != end; ++it)
            _taskQueue.push(std::move(*it));
    }

    bool empty() const { return _taskQueue.empty(); }

private:
    std::queue<QueueType> _taskQueue;
    std::mutex _taskQueueSync;
};

#ifdef __HX_SUPPORT_BOOST
template <typename QueueType>
class __QueueAdapterBoost {
public:
    typedef QueueType value_type;

    __QueueAdapterBoost() : _taskQueue(1024){};

    void push(QueueType item) { _taskQueue.push(std::move(item)); }

    bool pop(QueueType &result) { return _taskQueue.pop(result); }

    template <typename IteratorBegin,
              typename IteratorEnd,
              typename = typename std::enable_if<std::is_convertible<
                  typename std::iterator_traits<IteratorBegin>::value_type,
                  QueueType>::value>::type>
    void push_many(IteratorBegin begin, IteratorEnd end) {
        for (IteratorBegin it = begin; it != end; ++it) {
            _taskQueue.push(std::move(*it));
        }
    }

    bool empty() const { return _taskQueue.empty(); }

private:
    boost::lockfree::queue<QueueType> _taskQueue;
};

template <typename T>
class __QueueAdapterBoost<std::unique_ptr<T>> {
public:
    typedef std::unique_ptr<T> value_type;
    __QueueAdapterBoost() : _taskQueue(1024){};
    ~__QueueAdapterBoost() {
        T *pointer;
        while (!_taskQueue.empty()) {
            _taskQueue.pop(pointer);
            delete pointer;
        }
    }

    void push(std::unique_ptr<T> item) { _taskQueue.push(item.release()); }

    bool pop(std::unique_ptr<T> &result) {
        T *pointer;
        if (_taskQueue.pop(pointer)) {
            result.reset(pointer);
            return true;
        }

        return false;
    }

    template <typename IteratorBegin,
              typename IteratorEnd,
              typename = typename std::enable_if<std::is_convertible<
                  typename std::iterator_traits<IteratorBegin>::value_type,
                  std::unique_ptr<T>>::value>::type>
    void push_many(IteratorBegin begin, IteratorEnd end) {
        for (IteratorBegin it = begin; it != end; ++it) {
            _taskQueue.push(it->release());
        }
    }

    bool empty() const { return _taskQueue.empty(); }

private:
    boost::lockfree::queue<T *> _taskQueue;
};
#endif
#ifdef __HX_SUPPORT_TBB
template <typename QueueType>
class __QueueAdapterTBB {
public:
    typedef QueueType value_type;
    void push(QueueType item) { _taskQueue.push(std::move(item)); }

    bool pop(QueueType &result) { return _taskQueue.try_pop(result); }

    template <typename IteratorBegin,
              typename IteratorEnd,
              typename = typename std::enable_if<std::is_convertible<
                  typename std::iterator_traits<IteratorBegin>::value_type,
                  QueueType>::value>::type>
    void push_many(IteratorBegin begin, IteratorEnd end) {
        for (IteratorBegin it = begin; it != end; ++it) {
            _taskQueue.push(std::move(*it));
        }
    }

    bool empty() const { return _taskQueue.empty(); }

private:
    tbb::concurrent_queue<QueueType> _taskQueue;
};
#endif
}// namespace __internal

template <typename TaskQueue = hx::__internal::__QueueAdapterStd<
              std::unique_ptr<hx::__internal::__PoolTaskBase>>>
class ThreadPool {
public:
    ThreadPool(std::size_t threadpool_size = std::thread::hardware_concurrency())
        : _isRunning(true) {
        _threadPool.reserve(threadpool_size);
        for (std::size_t i = 0; i < threadpool_size; ++i)
            _threadPool.emplace_back(&ThreadPool::_ThreadRoutine, this);
    }

    ~ThreadPool() {
        _isRunning = false;
        _threadBarrierVar.notify_all();
        for (std::thread &t : _threadPool) {
            if (t.joinable()) t.join();
        }
    }

    std::size_t size() const { return _threadPool.size(); }

    template <typename Function,
              typename... Args,
              typename Result_t =
                  std::invoke_result_t<std::decay_t<Function>, std::decay_t<Args>...>>
    std::future<Result_t> async_task(Function &&f, Args... args) {
        std::promise<Result_t> result;
        std::future<Result_t> result_future = result.get_future();

        std::unique_ptr<hx::__internal::__PoolTaskBase> task =
            std::make_unique<hx::__internal::__PromiseTask<Function, Args...>>(
                std::move(result),
                std::forward<Function>(f),
                std::forward<Args>(args)...);
        _taskQueue.push(std::move(task));
        _threadBarrierVar.notify_one();
        return result_future;
    }

    template <
        typename Function,
        typename IteratorBegin,
        typename IteratorEnd,
        typename... Args,
        typename IteratorType = typename std::iterator_traits<IteratorBegin>::pointer,
        typename Result_t = std::invoke_result_t<std::decay_t<Function>,
                                                 std::size_t,
                                                 IteratorType,
                                                 std::decay_t<Args>...>>
    std::future<std::vector<Result_t>> async_map(Function &&f,
                                                 IteratorBegin begin,
                                                 IteratorEnd end,
                                                 Args... args) {
        std::vector<std::future<Result_t>> future_to_process;
        std::vector<std::unique_ptr<hx::__internal::__PoolTaskBase>> task_to_process;
        std::size_t enumeration = 0;

        for (IteratorBegin it = begin; it != end; ++it, ++enumeration) {
            std::promise<Result_t> result;
            std::future<Result_t> result_future = result.get_future();

            std::unique_ptr<hx::__internal::__PoolTaskBase> task = std::make_unique<
                hx::__internal::
                    __PromiseTask<Function, std::size_t, IteratorType, Args...>>(
                std::move(result),
                std::forward<Function>(f),
                std::forward<std::size_t>(enumeration),
                std::forward<IteratorType>(&(*it)),
                std::forward<Args>(args)...);
            task_to_process.push_back(std::move(task));
            future_to_process.push_back(std::move(result_future));
        }

        _taskQueue.push_many(task_to_process.begin(), task_to_process.end());
        _threadBarrierVar.notify_all();

        return std::async(
            std::launch::deferred,
            [](std::vector<std::future<Result_t>> data) -> std::vector<Result_t> {
                std::vector<Result_t> result;
                result.reserve(data.size());

                for (std::future<Result_t> &r : data)
                    result.push_back(std::move(r.get()));
                return result;
            },
            std::move(future_to_process));
    }

private:
    void _ThreadRoutine() noexcept {
        while (_DispatchTask()) {
            typename TaskQueue::value_type task;
            if (_taskQueue.pop(task)) (*task)();
        }
    }

    bool _DispatchTask() {
        std::unique_lock lock(_threadBarrierSync);
        _threadBarrierVar.wait(lock, [this]() -> bool {
            return !this->_isRunning || !this->_taskQueue.empty();
        });
        return _isRunning || !_taskQueue.empty();
    }

    TaskQueue _taskQueue;
    bool _isRunning;

    std::vector<std::thread> _threadPool;
    std::mutex _threadBarrierSync;
    std::condition_variable _threadBarrierVar;
};

#ifdef __HX_SUPPORT_BOOST
using ThreadPoolBoost = hx::ThreadPool<
    hx::__internal::__QueueAdapterBoost<std::unique_ptr<hx::__internal::__PoolTaskBase>>>;
#endif
#ifdef __HX_SUPPORT_TBB
using ThreadPoolTBB = hx::ThreadPool<
    hx::__internal::__QueueAdapterTBB<std::unique_ptr<hx::__internal::__PoolTaskBase>>>;
#endif
}// namespace hx

#endif