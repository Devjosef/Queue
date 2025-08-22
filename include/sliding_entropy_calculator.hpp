#ifndef SLIDING_ENTROPY_CALCULATOR_HPP
#define SLIDING_ENTROPY_CALCULATOR_HPP

#include "market_data.hpp"
#include <deque>
#include <array>
#include <atomic>
#include <chrono>
#include <vector>
#include <cmath>
#include <mutex>

class SlidingEntropyCalculator {
public:
    explicit SlidingEntropyCalculator(size_t window_size = 100, 
                                     size_t min_window = 50,
                                     size_t max_window = 500)
        : window_size_(window_size)
        , min_window_(min_window)
        , max_window_(max_window)
        , current_entropy_(0.0)
        , previous_entropy_(0.0)
        , action_counts_{0, 0, 0}
        , total_actions_(0)
        , last_update_time_(std::chrono::high_resolution_clock::now())
    {}

    void add_action(TraderAction action) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto now = std::chrono::high_resolution_clock::now();
        
        if (window_.size() >= window_size_) {
            remove_oldest_action();
        }
        
        window_.push_back(action);
        action_counts_[static_cast<size_t>(action)]++;
        total_actions_++;
        
        update_entropy_incremental();
        last_update_time_ = now;
        
        adapt_window_size();
    }

    void add_actions_batch(const std::vector<TraderAction>& actions) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        for (const auto& action : actions) {
            if (window_.size() >= window_size_) {
                remove_oldest_action();
            }
            
            window_.push_back(action);
            action_counts_[static_cast<size_t>(action)]++;
            total_actions_++;
        }
        
        update_entropy_incremental();
        adapt_window_size();
    }

    double get_current_entropy() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return current_entropy_;
    }

    double get_entropy_change_rate() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_update_time_).count();
        
        if (duration == 0) return 0.0;
        
        return (current_entropy_ - previous_entropy_) / (duration / 1000.0);
    }

    size_t get_window_size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return window_.size();
    }

    const std::array<uint32_t, 3>& get_action_distribution() const {
        return action_counts_;
    }

    bool is_high_entropy() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return current_entropy_ > 1.2;
    }

    bool is_low_entropy() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return current_entropy_ < 0.5;
    }

    bool is_medium_entropy() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return current_entropy_ >= 0.5 && current_entropy_ <= 1.2;
    }

    void set_window_size(size_t size) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (size >= min_window_ && size <= max_window_) {
            window_size_ = size;
            trim_window();
        }
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        window_.clear();
        action_counts_ = {0, 0, 0};
        total_actions_ = 0;
        current_entropy_ = 0.0;
        previous_entropy_ = 0.0;
    }

    std::vector<TraderAction> get_window_actions() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return std::vector<TraderAction>(window_.begin(), window_.end());
    }

private:
    // Remove the oldest action from the sliding window
    void remove_oldest_action() {
        if (window_.empty()) return;
        
        TraderAction oldest = window_.front();
        window_.pop_front();
        
        action_counts_[static_cast<size_t>(oldest)]--;
        total_actions_--;
    }

    // Update entropy calculation based on current action distribution
    void update_entropy_incremental() {
        previous_entropy_ = current_entropy_;
        
        if (total_actions_ == 0) {
            current_entropy_ = 0.0;
            return;
        }
        
        double entropy = 0.0;
        for (size_t i = 0; i < 3; ++i) {
            if (action_counts_[i] > 0) {
                double p = static_cast<double>(action_counts_[i]) / total_actions_;
                entropy -= p * std::log2(p);
            }
        }
        
        current_entropy_ = entropy;
    }

    // Adapt window size based on entropy change rate
    void adapt_window_size() {
        double entropy_change = std::abs(current_entropy_ - previous_entropy_);
        
        if (entropy_change > 0.1 && window_.size() < max_window_) {
            window_size_ = std::min(window_.size() + 10, max_window_);
        } else if (entropy_change < 0.01 && window_.size() > min_window_) {
            window_size_ = std::max(window_.size() - 5, min_window_);
        }
    }

    // Trim window to current window size
    void trim_window() {
        while (window_.size() > window_size_) {
            remove_oldest_action();
        }
    }

    // Configuration parameters
    size_t window_size_;
    size_t min_window_;
    size_t max_window_;
    
    // Thread-safe state
    mutable std::mutex mutex_;
    std::deque<TraderAction> window_;
    std::array<uint32_t, 3> action_counts_;
    uint32_t total_actions_;
    double current_entropy_;
    double previous_entropy_;
    std::chrono::high_resolution_clock::time_point last_update_time_;
};

#endif // SLIDING_ENTROPY_CALCULATOR_HPP
