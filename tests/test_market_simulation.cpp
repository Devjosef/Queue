#include "market_pipeline.hpp"
#include "sliding_entropy_calculator.hpp"
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <cassert>

class MarketSimulator {
public:
    void run_market_simulation() {
        std::cout << "Real Market Data Simulation Tests\n";
        std::cout << "================================\n\n";
        
        simulate_bull_market();
        simulate_bear_market();
        simulate_market_crash();
        simulate_normal_trading();
        simulate_high_frequency_trading();
        simulate_market_recovery();
        
        std::cout << "\nAll market simulation tests completed!\n";
    }

private:
    void simulate_bull_market() {
        std::cout << "1. Bull Market Simulation\n";
        std::cout << "-------------------------\n";
        
        SlidingEntropyCalculator calc(100);
        std::vector<TraderAction> actions = generate_bull_market_data(1000);
        
        for (const auto& action : actions) {
            calc.add_action(action);
        }
        
        double entropy = calc.get_current_entropy();
        auto distribution = calc.get_action_distribution();
        
        std::cout << "Bull Market Results:\n";
        std::cout << "  Entropy: " << std::fixed << std::setprecision(4) << entropy << " bits\n";
        std::cout << "  Distribution: HOLD=" << distribution[0] 
                  << ", BUY=" << distribution[1] 
                  << ", SELL=" << distribution[2] << "\n";
        std::cout << "  Classification: " << classify_entropy(entropy) << "\n";
        
        // Bull market should show moderate entropy with more buys
        assert(distribution[1] > distribution[2]); // More buys than sells
        assert(entropy > 0.5); // Moderate to high entropy
        
        std::cout << "✓ Bull market simulation passed\n\n";
    }

    void simulate_bear_market() {
        std::cout << "2. Bear Market Simulation\n";
        std::cout << "-------------------------\n";
        
        SlidingEntropyCalculator calc(100);
        std::vector<TraderAction> actions = generate_bear_market_data(1000);
        
        for (const auto& action : actions) {
            calc.add_action(action);
        }
        
        double entropy = calc.get_current_entropy();
        auto distribution = calc.get_action_distribution();
        
        std::cout << "Bear Market Results:\n";
        std::cout << "  Entropy: " << std::fixed << std::setprecision(4) << entropy << " bits\n";
        std::cout << "  Distribution: HOLD=" << distribution[0] 
                  << ", BUY=" << distribution[1] 
                  << ", SELL=" << distribution[2] << "\n";
        std::cout << "  Classification: " << classify_entropy(entropy) << "\n";
        
        // Bear market should show moderate entropy with more sells
        assert(distribution[2] > distribution[1]); // More sells than buys
        assert(entropy > 0.5); // Moderate to high entropy
        
        std::cout << "✓ Bear market simulation passed\n\n";
    }

    void simulate_market_crash() {
        std::cout << "3. Market Crash Simulation\n";
        std::cout << "--------------------------\n";
        
        SlidingEntropyCalculator calc(100);
        std::vector<TraderAction> actions = generate_market_crash_data(1000);
        
        for (const auto& action : actions) {
            calc.add_action(action);
        }
        
        double entropy = calc.get_current_entropy();
        auto distribution = calc.get_action_distribution();
        
        std::cout << "Market Crash Results:\n";
        std::cout << "  Entropy: " << std::fixed << std::setprecision(4) << entropy << " bits\n";
        std::cout << "  Distribution: HOLD=" << distribution[0] 
                  << ", BUY=" << distribution[1] 
                  << ", SELL=" << distribution[2] << "\n";
        std::cout << "  Classification: " << classify_entropy(entropy) << "\n";
        
        // Market crash should show low entropy with panic selling
        assert(distribution[2] >> distribution[1]); // Much more sells than buys
        assert(entropy < 0.9); // Low to moderate entropy (panic behavior)
        
        std::cout << "✓ Market crash simulation passed\n\n";
    }

    void simulate_normal_trading() {
        std::cout << "4. Normal Trading Simulation\n";
        std::cout << "----------------------------\n";
        
        SlidingEntropyCalculator calc(100);
        std::vector<TraderAction> actions = generate_normal_trading_data(1000);
        
        for (const auto& action : actions) {
            calc.add_action(action);
        }
        
        double entropy = calc.get_current_entropy();
        auto distribution = calc.get_action_distribution();
        
        std::cout << "Normal Trading Results:\n";
        std::cout << "  Entropy: " << std::fixed << std::setprecision(4) << entropy << " bits\n";
        std::cout << "  Distribution: HOLD=" << distribution[0] 
                  << ", BUY=" << distribution[1] 
                  << ", SELL=" << distribution[2] << "\n";
        std::cout << "  Classification: " << classify_entropy(entropy) << "\n";
        
        // Normal trading should show high entropy with balanced distribution
        assert(entropy > 1.0); // High entropy (diverse behavior)
        assert(std::abs(static_cast<int>(distribution[1]) - static_cast<int>(distribution[2])) < 100); // Roughly balanced
        
        std::cout << "✓ Normal trading simulation passed\n\n";
    }

    void simulate_high_frequency_trading() {
        std::cout << "5. High-Frequency Trading Simulation\n";
        std::cout << "------------------------------------\n";
        
        MarketPipeline pipeline(10000, 100, 50);
        pipeline.start(2, 2);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Simulate HFT with rapid, small trades
        for (int i = 0; i < 5000; ++i) {
            MarketData data;
            // HFT typically has many small, rapid trades
            data.add_action(static_cast<TraderAction>(i % 3));
            pipeline.feed_market_data(data);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        const auto& metrics = pipeline.get_metrics();
        double entropy = pipeline.get_current_entropy();
        
        std::cout << "HFT Simulation Results:\n";
        std::cout << "  Processing time: " << duration << " ms\n";
        std::cout << "  Throughput: " << (5000.0 / duration) * 1000 << " packets/sec\n";
        std::cout << "  Average latency: " << metrics.average_latency_ns.load() / 1000.0 << " μs\n";
        std::cout << "  Final entropy: " << std::fixed << std::setprecision(4) << entropy << " bits\n";
        std::cout << "  Queue full events: " << metrics.queue_full_count.load() << "\n";
        
        assert(metrics.total_processed.load() > 0);
        assert(entropy > 0.0);
        
        pipeline.stop();
        std::cout << "✓ High-frequency trading simulation passed\n\n";
    }

    void simulate_market_recovery() {
        std::cout << "6. Market Recovery Simulation\n";
        std::cout << "-----------------------------\n";
        
        SlidingEntropyCalculator calc(100);
        
        // Phase 1: Crash (low entropy)
        std::vector<TraderAction> crash_actions = generate_market_crash_data(300);
        for (const auto& action : crash_actions) {
            calc.add_action(action);
        }
        
        double crash_entropy = calc.get_current_entropy();
        std::cout << "Crash Phase Entropy: " << std::fixed << std::setprecision(4) << crash_entropy << " bits\n";
        
        // Phase 2: Recovery (increasing entropy)
        std::vector<TraderAction> recovery_actions = generate_normal_trading_data(700);
        for (const auto& action : recovery_actions) {
            calc.add_action(action);
        }
        
        double recovery_entropy = calc.get_current_entropy();
        std::cout << "Recovery Phase Entropy: " << std::fixed << std::setprecision(4) << recovery_entropy << " bits\n";
        
        // Recovery should show higher entropy than crash
        assert(recovery_entropy > crash_entropy);
        assert(recovery_entropy > 0.5); // Should recover to moderate/high entropy
        
        std::cout << "✓ Market recovery simulation passed\n\n";
    }

    // Market data generators
    std::vector<TraderAction> generate_bull_market_data(int count) {
        std::vector<TraderAction> actions;
        actions.reserve(count);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        
        for (int i = 0; i < count; ++i) {
            double rand = dis(gen);
            if (rand < 0.15) {
                actions.push_back(TraderAction::HOLD);
            } else if (rand < 0.75) {
                actions.push_back(TraderAction::BUY); // 60% buys
            } else {
                actions.push_back(TraderAction::SELL); // 25% sells
            }
        }
        
        return actions;
    }

    std::vector<TraderAction> generate_bear_market_data(int count) {
        std::vector<TraderAction> actions;
        actions.reserve(count);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        
        for (int i = 0; i < count; ++i) {
            double rand = dis(gen);
            if (rand < 0.15) {
                actions.push_back(TraderAction::HOLD);
            } else if (rand < 0.35) {
                actions.push_back(TraderAction::BUY); // 20% buys
            } else {
                actions.push_back(TraderAction::SELL); // 65% sells
            }
        }
        
        return actions;
    }

    std::vector<TraderAction> generate_market_crash_data(int count) {
        std::vector<TraderAction> actions;
        actions.reserve(count);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        
        for (int i = 0; i < count; ++i) {
            double rand = dis(gen);
            if (rand < 0.02) {
                actions.push_back(TraderAction::HOLD);
            } else if (rand < 0.05) {
                actions.push_back(TraderAction::BUY); // 3% buys
            } else {
                actions.push_back(TraderAction::SELL); // 95% sells (panic)
            }
        }
        
        return actions;
    }

    std::vector<TraderAction> generate_normal_trading_data(int count) {
        std::vector<TraderAction> actions;
        actions.reserve(count);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        
        for (int i = 0; i < count; ++i) {
            double rand = dis(gen);
            if (rand < 0.33) {
                actions.push_back(TraderAction::HOLD);
            } else if (rand < 0.66) {
                actions.push_back(TraderAction::BUY);
            } else {
                actions.push_back(TraderAction::SELL);
            }
        }
        
        return actions;
    }

    std::string classify_entropy(double entropy) {
        if (entropy < 0.5) return "Low (predictable behavior)";
        if (entropy < 1.2) return "Medium (mixed behavior)";
        return "High (unpredictable behavior)";
    }
};

int main() {
    MarketSimulator simulator;
    simulator.run_market_simulation();
    return 0;
}
