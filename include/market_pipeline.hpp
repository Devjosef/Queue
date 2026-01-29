#ifndef MARKET_PIPELINE_HPP
#define MARKET_PIPELINE_HPP

#include "optimized_queue.hpp"
#include "sliding_entropy_calculator.hpp"
#include "market_data.hpp"
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>
#include <functional>

struct PipelineMetrics {
    std::atomic<uint64_t> total_processed{0};
    std::atomic<uint64_t> queue_full_count{0};
    std::atomic<uint64_t> backpressure_events{0};
    std::atomic<double> average_latency_ns{0.0};
    std::atomic<double> max_latency_ns{0.0};
    std::atomic<uint64_t> entropy_updates{0};
    std::atomic<double> current_entropy{0.0};
    std::atomic<double> entropy_change_rate{0.0};
};

class MarketPipeline {
public:
    using EntropyCallback = std::function<void(double, double)>;
    
    MarketPipeline(size_t queue_capacity = 10000,
                   size_t batch_size = 100,
                   size_t window_size = 100)
        : queue_(queue_capacity, batch_size)
        , entropy_calc_(window_size)
        , queue_capacity_(queue_capacity)
        , running_(false)
        , producer_threads_()
        , consumer_threads_()
        , entropy_callback_(nullptr)
        , metrics_()
    {}

    ~MarketPipeline() {
        stop();
    }

    void start(size_t num_producers = 2, size_t num_consumers = 1) {
        if (running_.load()) return;
        
        running_.store(true);
        
        for (size_t i = 0; i < num_producers; ++i) {
            producer_threads_.emplace_back(&MarketPipeline::producer_loop, this, i);
        }
        
        for (size_t i = 0; i < num_consumers; ++i) {
            consumer_threads_.emplace_back(&MarketPipeline::consumer_loop, this, i);
        }
    }

    void stop() {
        running_.store(false);
        
        // Notify any waiting consumers to wake up
        queue_.notify_all();
        
        for (auto& thread : producer_threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        
        for (auto& thread : consumer_threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        
        producer_threads_.clear();
        consumer_threads_.clear();
    }

    bool feed_market_data(const MarketData& data) {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        bool success = queue_.push(data);
        
        if (success) {
            auto end_time = std::chrono::high_resolution_clock::now();
            auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();

            // Account the processed item first so latency-update calculations
            // which rely on total_processed do not divide by zero.
            metrics_.total_processed.fetch_add(1);
            update_latency_metrics(latency);
        } else {
            metrics_.queue_full_count.fetch_add(1);
            
            size_t current_size = queue_.size();
            if (current_size >= queue_capacity_ * 0.9) {
                metrics_.backpressure_events.fetch_add(1);
                queue_.wait_for_backpressure();
            }
        }
        
        return success;
    }

    void set_entropy_callback(EntropyCallback callback) {
        entropy_callback_ = callback;
    }

    const PipelineMetrics& get_metrics() const {
        return metrics_;
    }

    double get_current_entropy() const {
        return entropy_calc_.get_current_entropy();
    }

    double get_entropy_change_rate() const {
        return entropy_calc_.get_entropy_change_rate();
    }

    size_t get_queue_size() const {
        return queue_.size();
    }

    bool is_high_entropy() const {
        return entropy_calc_.is_high_entropy();
    }

    bool is_low_entropy() const {
        return entropy_calc_.is_low_entropy();
    }

    void set_queue_capacity(size_t capacity) {
        queue_capacity_ = capacity;
        queue_.set_capacity(capacity);
    }

    void set_batch_size(size_t batch_size) {
        queue_.set_batch_size(batch_size);
    }

    void set_window_size(size_t window_size) {
        entropy_calc_.set_window_size(window_size);
    }

private:
    void producer_loop(size_t id) {
        static double last_price = 695.49;

    while (running_.load()) {
        double spy_price = get_spy_price();
        double dp = (spy_price - last_price) / last_price * 100.0;
        
        TraderAction action = get_spy_action(dp);

        MarketData data;
        data.add_action(action);
        feed_market_data(data);

        last_price = spy_price;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    void consumer_loop(size_t id) {
        std::vector<MarketData> batch;
        
        while (running_.load()) {
            if (queue_.try_pop_batch(batch)) {
                process_batch(batch);
            } else {
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        }
    }

    void process_batch(const std::vector<MarketData>& batch) {
        for (const auto& data : batch) {
            const auto& actions = data.get_actions();
            
            for (const auto& action : actions) {
                entropy_calc_.add_action(action);
                metrics_.entropy_updates.fetch_add(1);
            }
        }
        
        double current_entropy = entropy_calc_.get_current_entropy();
        double change_rate = entropy_calc_.get_entropy_change_rate();
        
        metrics_.current_entropy.store(current_entropy);
        metrics_.entropy_change_rate.store(change_rate);
        
        if (entropy_callback_) {
            entropy_callback_(current_entropy, change_rate);
        }
    }

    void update_latency_metrics(uint64_t latency_ns) {
        // Use atomic operations to avoid race conditions
        uint64_t total = metrics_.total_processed.load();
        
        // Update max latency atomically
        double current_max = metrics_.max_latency_ns.load();
        while (latency_ns > current_max && 
               !metrics_.max_latency_ns.compare_exchange_weak(current_max, latency_ns)) {
            // Retry if compare_exchange failed
        }
        
        // Update average latency using atomic operations
        double current_avg = metrics_.average_latency_ns.load();
        double new_avg = (current_avg * (total - 1) + latency_ns) / total;
        
        // Use compare_exchange to ensure atomic update
        while (!metrics_.average_latency_ns.compare_exchange_weak(current_avg, new_avg)) {
            // Recalculate if value changed during update
            total = metrics_.total_processed.load();
            new_avg = (current_avg * (total - 1) + latency_ns) / total;
        }
    }

    OptimizedQueue<MarketData> queue_;
    SlidingEntropyCalculator entropy_calc_;
    size_t queue_capacity_;
    std::atomic<bool> running_;
    std::vector<std::thread> producer_threads_;
    std::vector<std::thread> consumer_threads_;
    EntropyCallback entropy_callback_;
    PipelineMetrics metrics_;
};

#endif // MARKET_PIPELINE_HPP
