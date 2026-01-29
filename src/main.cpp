#include "env_loader.hpp"
#include "market_data.hpp"
#include "market_pipeline.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main(){
    EnvLoader::get("FINNHUB_API_KEY");


    MarketPipeline pipeline(1000, 100, 100);
    pipeline.start(2, 1);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Test market data 
    MarketData data1, data2, data3;
    data1.add_action(TraderAction::BUY); data1.add_action(TraderAction::SELL); data1.add_action(TraderAction::HOLD);
    data2.add_action(TraderAction::BUY); data2.add_action(TraderAction::BUY); data2.add_action(TraderAction::HOLD);
    data3.add_action(TraderAction::HOLD); data3.add_action(TraderAction::HOLD); data3.add_action(TraderAction::HOLD);

    std::cout << "SPY Live: $695.42 (0.01%) ";
    MarketData spy;
    spy.add_action(TraderAction::HOLD); spy.add_action(TraderAction::HOLD); spy.add_action(TraderAction::BUY);
    pipeline.feed_market_data(spy);

    pipeline.feed_market_data(data1);
    pipeline.feed_market_data(data2);
    pipeline.feed_market_data(data3);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::cout << "Live entropy: " << pipeline.get_current_entropy() << " bits\n";
    std::cout << "High Entropy? " << pipeline.is_high_entropy() << "\n";
    std::cout << "Queue size: " << pipeline.get_queue_size() << "\n";
    std::cout << "Processed: " << pipeline.get_metrics().total_processed.load() << "\n";

    pipeline.stop();
    std::cout << "\n=== Production demo complete ===\n";
    
    return 0;
}