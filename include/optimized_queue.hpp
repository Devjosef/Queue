#ifndef OPTIMIZED_QUEUE_HPP
#define OPTIMIZED_QUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <vector>
#include <memory>

template <typename T>
class OptimizedQueue {
public:
    explicit OptimizedQueue(size_t capacity = 10000, size_t batch_size = 100)
        : capacity_(capacity)
        , batch_size_(batch_size)
        , head_(std::make_unique<Node>())
        , tail_(head_.get())
        , size_(0)
        , backpressure_threshold_(capacity * 0.8)
    {}

    ~OptimizedQueue() = default;

    OptimizedQueue(const OptimizedQueue&) = delete;
    OptimizedQueue& operator=(const OptimizedQueue&) = delete;

    bool push(const T& data) {
        auto node = std::make_unique<Node>(data);
        
        {
            std::lock_guard<std::mutex> lock(tail_mutex_);
            
            if (size_.load() >= capacity_) {
                return false;
            }
            
            tail_->next = std::move(node);
            tail_ = tail_->next.get();
            size_.fetch_add(1);
        }
        
        cv_.notify_one();
        
        if (size_.load() >= backpressure_threshold_) {
            backpressure_cv_.notify_all();
        }
        
        return true;
    }

    bool try_pop(T& data) {
        std::lock_guard<std::mutex> lock(head_mutex_);
        
        if (head_->next == nullptr) {
            return false;
        }
        
        auto old_head = std::move(head_);
        head_ = std::move(old_head->next);
        data = head_->data;
        size_.fetch_sub(1);
        
        if (size_.load() < backpressure_threshold_) {
            backpressure_cv_.notify_all();
        }
        
        return true;
    }

    bool try_pop_batch(std::vector<T>& batch) {
        std::lock_guard<std::mutex> lock(head_mutex_);
        
        if (head_->next == nullptr) {
            return false;
        }
        
        batch.clear();
        batch.reserve(batch_size_);
        
        size_t count = 0;
        while (count < batch_size_ && head_->next != nullptr) {
            auto old_head = std::move(head_);
            head_ = std::move(old_head->next);
            batch.push_back(head_->data);
            count++;
        }
        
        size_.fetch_sub(count);
        
        if (size_.load() < backpressure_threshold_) {
            backpressure_cv_.notify_all();
        }
        
        return true;
    }

    void wait_and_pop(T& data) {
        std::unique_lock<std::mutex> lock(head_mutex_);
        cv_.wait(lock, [this] { return head_->next != nullptr; });
        
        auto old_head = std::move(head_);
        head_ = std::move(old_head->next);
        data = head_->data;
        size_.fetch_sub(1);
        
        if (size_.load() < backpressure_threshold_) {
            backpressure_cv_.notify_all();
        }
    }

    bool empty() const {
        return size_.load() == 0;
    }

    size_t size() const {
        return size_.load();
    }

    void wait_for_backpressure() {
        std::unique_lock<std::mutex> lock(head_mutex_);
        backpressure_cv_.wait(lock, [this] { return size_.load() < backpressure_threshold_; });
    }

    void set_capacity(size_t capacity) {
        capacity_ = capacity;
        backpressure_threshold_ = capacity * 0.8;
    }

    void set_batch_size(size_t batch_size) {
        batch_size_ = batch_size;
    }

    void notify_all() {
        cv_.notify_all();
        backpressure_cv_.notify_all();
    }

private:
    struct Node {
        T data;
        std::unique_ptr<Node> next;
        
        Node() : data(), next(nullptr) {}
        explicit Node(const T& d) : data(d), next(nullptr) {}
    };

    size_t capacity_;
    size_t batch_size_;
    std::mutex head_mutex_;
    std::mutex tail_mutex_;
    std::unique_ptr<Node> head_;
    Node* tail_;
    std::atomic<size_t> size_;
    size_t backpressure_threshold_;
    
    std::condition_variable cv_;
    std::condition_variable backpressure_cv_;
};

#endif // OPTIMIZED_QUEUE_HPP
