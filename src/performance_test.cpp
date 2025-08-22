#include "market_pipeline.hpp"
#include "optimized_queue.hpp"
#include "sliding_entropy_calculator.hpp"
#include <iostream>
#include <chrono>
#include <random>
#include <thread>
#include <vector>
#include <iomanip>

class PerformanceTest {
public:
    void run_all_tests() {
        std::cout << "Performance Testing Suite\n";
        std::cout << "========================\n\n";
        
        test_optimized_queue_performance();
        test_sliding_entropy_performance();
        test_market_pipeline_performance();
        test_real_market_distributions();
        test_backpressure_handling();
    }

private:
    void test_optimized_queue_performance() {
        std::cout << "1. Optimized Queue Performance Test\n";
        std::cout << "-----------------------------------\n";
        
        OptimizedQueue<int> queue(10000, 100);
        std::vector<int> batch;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < 100000; ++i) {
            queue.push(i);
        }
        
        auto mid = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < 100000; ++i) {
            int value;
            queue.try_pop(value);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        
        auto push_time = std::chrono::duration_cast<std::chrono::microseconds>(mid - start).count();
        auto pop_time = std::chrono::duration_cast<std::chrono::microseconds>(end - mid).count();
        
        std::cout << "Push operations: 100,000 in " << push_time << " μs\n";
        std::cout << "Pop operations: 100,000 in " << pop_time << " μs\n";
        std::cout << "Throughput: " << (200000.0 / (push_time + pop_time)) * 1000 << " ops/sec\n\n";
    }

    void test_sliding_entropy_performance() {
        std::cout << "2. Sliding Entropy Performance Test\n";
        std::cout << "-----------------------------------\n";
        
        SlidingEntropyCalculator calc(100);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 2);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < 100000; ++i) {
            calc.add_action(static_cast<TraderAction>(dis(gen)));
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        
        std::cout << "100,000 entropy updates in " << duration << " μs\n";
        std::cout << "Average latency: " << duration / 100000.0 << " μs per update\n";
        std::cout << "Final entropy: " << std::fixed << std::setprecision(6) << calc.get_current_entropy() << " bits\n";
        std::cout << "Window size: " << calc.get_window_size() << "\n\n";
    }

    void test_market_pipeline_performance() {
        std::cout << "3. Market Pipeline Performance Test\n";
        std::cout << "-----------------------------------\n";
        
        MarketPipeline pipeline(10000, 100, 100);
        
        pipeline.set_entropy_callback([](double entropy, double change_rate) {
            if (entropy > 1.0) {
                std::cout << "High entropy detected: " << std::fixed << std::setprecision(4) << entropy << " bits\n";
            }
        });
        
        pipeline.start(2, 1);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 2);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < 50000; ++i) {
            MarketData data;
            for (int j = 0; j < 5; ++j) {
                data.add_action(static_cast<TraderAction>(dis(gen)));
            }
            pipeline.feed_market_data(data);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        auto metrics = pipeline.get_metrics();
        
        std::cout << "50,000 market data packets processed in " << duration << " ms\n";
        std::cout << "Throughput: " << (50000.0 / duration) * 1000 << " packets/sec\n";
        std::cout << "Average latency: " << metrics.average_latency_ns.load() / 1000.0 << " μs\n";
        std::cout << "Max latency: " << metrics.max_latency_ns.load() / 1000.0 << " μs\n";
        std::cout << "Queue full events: " << metrics.queue_full_count.load() << "\n";
        std::cout << "Backpressure events: " << metrics.backpressure_events.load() << "\n";
        std::cout << "Final entropy: " << std::fixed << std::setprecision(4) << pipeline.get_current_entropy() << " bits\n\n";
        
        pipeline.stop();
    }

    void test_real_market_distributions() {
        std::cout << "4. Real Market Distribution Test\n";
        std::cout << "--------------------------------\n";
        
        SlidingEntropyCalculator calc(100);
        
        std::vector<std::vector<TraderAction>> scenarios = {
            // Bull market: mostly buys
            generate_skewed_distribution(0.6, 0.3, 0.1, 1000),
            // Bear market: mostly sells
            generate_skewed_distribution(0.1, 0.3, 0.6, 1000),
            // Market crash: panic selling
            generate_skewed_distribution(0.05, 0.15, 0.8, 1000),
            // Normal trading: balanced
            generate_skewed_distribution(0.33, 0.34, 0.33, 1000)
        };
        
        std::vector<std::string> scenario_names = {
            "Bull Market", "Bear Market", "Market Crash", "Normal Trading"
        };
        
        for (size_t i = 0; i < scenarios.size(); ++i) {
            calc.clear();
            
            for (const auto& action : scenarios[i]) {
                calc.add_action(action);
            }
            
            double entropy = calc.get_current_entropy();
            auto distribution = calc.get_action_distribution();
            
            std::cout << scenario_names[i] << ":\n";
            std::cout << "  Entropy: " << std::fixed << std::setprecision(4) << entropy << " bits\n";
            std::cout << "  Distribution: HOLD=" << distribution[0] 
                      << ", BUY=" << distribution[1] 
                      << ", SELL=" << distribution[2] << "\n";
            
            if (calc.is_high_entropy()) {
                std::cout << "  Classification: High entropy (unpredictable)\n";
            } else if (calc.is_low_entropy()) {
                std::cout << "  Classification: Low entropy (predictable)\n";
            } else {
                std::cout << "  Classification: Medium entropy (mixed)\n";
            }
            std::cout << "\n";
        }
    }

    void test_backpressure_handling() {
        std::cout << "5. Backpressure Handling Test\n";
        std::cout << "-----------------------------\n";
        
        MarketPipeline pipeline(100, 10, 50);
        pipeline.start(1, 1);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 2);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        int successful = 0;
        int failed = 0;
        
        for (int i = 0; i < 1000; ++i) {
            MarketData data;
            for (int j = 0; j < 10; ++j) {
                data.add_action(static_cast<TraderAction>(dis(gen)));
            }
            
            if (pipeline.feed_market_data(data)) {
                successful++;
            } else {
                failed++;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        auto metrics = pipeline.get_metrics();
        
        std::cout << "Backpressure test results:\n";
        std::cout << "  Successful feeds: " << successful << "\n";
        std::cout << "  Failed feeds: " << failed << "\n";
        std::cout << "  Queue full events: " << metrics.queue_full_count.load() << "\n";
        std::cout << "  Backpressure events: " << metrics.backpressure_events.load() << "\n";
        std::cout << "  Processing time: " << duration << " ms\n\n";
        
        pipeline.stop();
    }

    std::vector<TraderAction> generate_skewed_distribution(double hold_prob, double buy_prob, double sell_prob, int count) {
        std::vector<TraderAction> actions;
        actions.reserve(count);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        
        for (int i = 0; i < count; ++i) {
            double rand = dis(gen);
            if (rand < hold_prob) {
                actions.push_back(TraderAction::HOLD);
            } else if (rand < hold_prob + buy_prob) {
                actions.push_back(TraderAction::BUY);
            } else {
                actions.push_back(TraderAction::SELL);
            }
        }
        
        return actions;
    }
};

int main() {
    PerformanceTest test;
    test.run_all_tests();
    return 0;
}
