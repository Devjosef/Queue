#include "concurrent_queue.hpp"
#include "market_data.hpp"
#include "entropy_calculator.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Queue-Based Concurrency Solution for Shannon Entropy Market Analysis\n";
    std::cout << "==================================================================\n\n";

    ConcurrentQueue<MarketData> market_queue;
    EntropyCalculator entropy_calc;

    std::cout << "Testing TraderAction enum values:\n";
    std::cout << "HOLD = " << static_cast<int>(TraderAction::HOLD) << "\n";
    std::cout << "BUY = " << static_cast<int>(TraderAction::BUY) << "\n";
    std::cout << "SELL = " << static_cast<int>(TraderAction::SELL) << "\n\n";

    std::cout << "Testing queue operations:\n";

    MarketData data1;
    data1.add_action(TraderAction::BUY);
    data1.add_action(TraderAction::SELL);
    data1.add_action(TraderAction::HOLD);

    MarketData data2;
    data2.add_action(TraderAction::BUY);
    data2.add_action(TraderAction::BUY);
    data2.add_action(TraderAction::SELL);

    market_queue.push(data1);
    market_queue.push(data2);

    std::cout << "Queue size after pushing: " << (market_queue.empty() ? "empty" : "not empty") << "\n";

    MarketData received_data;
    if (market_queue.try_pop(received_data)) {
        std::cout << "Popped data with " << received_data.get_actions().size() << " actions\n";
        
        double entropy = entropy_calc.calculate_entropy(received_data.get_actions());
        std::cout << "Entropy: " << entropy << " bits\n";
        
        if (entropy_calc.is_high_entropy(entropy)) {
            std::cout << "High entropy detected (> 1.2 bits)\n";
        } else if (entropy_calc.is_low_entropy(entropy)) {
            std::cout << "Low entropy detected (< 0.5 bits)\n";
        } else {
            std::cout << "Medium entropy detected (0.5-1.2 bits)\n";
        }
    }

    std::cout << "\nTest completed successfully!\n";
    return 0;
}
