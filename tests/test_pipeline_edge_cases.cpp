#include "market_pipeline.hpp"
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include <atomic>

void test_empty_pipeline() {
    std::cout << "Testing empty pipeline...\n";
    
    MarketPipeline pipeline(10, 2, 5);
    
    assert(pipeline.get_queue_size() == 0);
    assert(pipeline.get_current_entropy() == 0.0);
    
    std::cout << "✓ Empty pipeline passed\n";
}

void test_pipeline_no_consumers() {
    std::cout << "Testing pipeline with no consumers...\n";
    
    MarketPipeline pipeline(5, 2, 3);
    pipeline.start(1, 0); // No consumers
    
    MarketData data;
    data.add_action(TraderAction::BUY);
    
    // Should succeed (queue has space)
    assert(pipeline.feed_market_data(data));
    
    // But entropy should remain 0 (no processing)
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    assert(pipeline.get_current_entropy() == 0.0);
    
    pipeline.stop();
    std::cout << "✓ Pipeline with no consumers passed\n";
}

void test_pipeline_overflow() {
    std::cout << "Testing pipeline overflow handling...\n";
    
    MarketPipeline pipeline(2, 1, 3);
    pipeline.start(1, 1);
    
    MarketData data;
    data.add_action(TraderAction::BUY);
    
    // Fill queue
    assert(pipeline.feed_market_data(data));
    assert(pipeline.feed_market_data(data));
    
    // Should fail (queue full)
    assert(!pipeline.feed_market_data(data));
    
    pipeline.stop();
    std::cout << "✓ Pipeline overflow handling passed\n";
}

void test_rapid_start_stop() {
    std::cout << "Testing rapid start/stop cycles...\n";
    
    MarketPipeline pipeline(10, 2, 5);
    
    for (int i = 0; i < 10; ++i) {
        pipeline.start(1, 1);
        pipeline.stop();
    }
    
    std::cout << "✓ Rapid start/stop cycles passed\n";
}

void test_callback_functionality() {
    std::cout << "Testing callback functionality...\n";
    
    std::atomic<int> callback_count{0};
    MarketPipeline pipeline(10, 2, 5);
    
    pipeline.set_entropy_callback([&callback_count](double entropy, double rate) {
        callback_count.fetch_add(1);
    });
    
    pipeline.start(1, 1);
    
    MarketData data;
    data.add_action(TraderAction::BUY);
    data.add_action(TraderAction::SELL);
    
    pipeline.feed_market_data(data);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    assert(callback_count.load() > 0);
    
    pipeline.stop();
    std::cout << "✓ Callback functionality: " << callback_count.load() << " calls\n";
}

void test_pipeline_metrics() {
    std::cout << "Testing pipeline metrics...\n";
    
    MarketPipeline pipeline(100, 10, 20);
    pipeline.start(1, 1);
    
    MarketData data;
    data.add_action(TraderAction::BUY);
    data.add_action(TraderAction::SELL);
    
    for (int i = 0; i < 50; ++i) {
        pipeline.feed_market_data(data);
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    const auto& metrics = pipeline.get_metrics();
    assert(metrics.total_processed.load() > 0);
    assert(metrics.entropy_updates.load() > 0);
    
    pipeline.stop();
    std::cout << "✓ Pipeline metrics: " << metrics.total_processed.load() << " processed\n";
}

int main() {
    std::cout << "Pipeline Edge Case Tests\n";
    std::cout << "=======================\n\n";
    
    test_empty_pipeline();
    test_pipeline_no_consumers();
    test_pipeline_overflow();
    test_rapid_start_stop();
    test_callback_functionality();
    test_pipeline_metrics();
    
    std::cout << "\nAll pipeline edge case tests passed!\n";
    return 0;
}
