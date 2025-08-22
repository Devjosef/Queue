#ifndef CONCURRENT_QUEUE_HPP
#define CONCURRENT_QUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template <typename T>
class ConcurrentQueue {
    public:
        ConcurrentQueue() = default;
        ~ConcurrentQueue() = default;

        // Disable copy semantics
        ConcurrentQueue(const ConcurrentQueue&) = delete;
        ConcurrentQueue& operator=(const ConcurrentQueue&) = delete;

        void push(const T& value);

        bool try_pop(T& result);
        
        void wait_and_pop(T& result);
        
        bool empty() const;

    private:
        mutable std::mutex m_mutex;
        std::queue<T> m_queue;
        std::condition_variable m_cond_var;
};

#include "concurrent_queue.tpp"


#endif // CONCURRENT_QUEUE_HPP
