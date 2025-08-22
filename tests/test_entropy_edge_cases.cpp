#include "sliding_entropy_calculator.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

void test_empty_window_entropy() {
    std::cout << "Testing empty window entropy...\n";
    
    SlidingEntropyCalculator calc(10);
    
    assert(calc.get_current_entropy() == 0.0);
    assert(calc.get_window_size() == 0);
    assert(calc.is_low_entropy());
    
    std::cout << "✓ Empty window entropy passed\n";
}

void test_single_action_type() {
    std::cout << "Testing single action type (zero entropy)...\n";
    
    SlidingEntropyCalculator calc(5);
    
    for (int i = 0; i < 5; ++i) {
        calc.add_action(TraderAction::BUY);
    }
    
    assert(calc.get_current_entropy() == 0.0); // All same actions
    assert(calc.is_low_entropy());
    
    std::cout << "✓ Single action type passed\n";
}

void test_maximum_entropy() {
    std::cout << "Testing maximum entropy (equal distribution)...\n";
    
    SlidingEntropyCalculator calc(6);
    
    calc.add_action(TraderAction::HOLD);
    calc.add_action(TraderAction::BUY);
    calc.add_action(TraderAction::SELL);
    calc.add_action(TraderAction::HOLD);
    calc.add_action(TraderAction::BUY);
    calc.add_action(TraderAction::SELL);
    
    double entropy = calc.get_current_entropy();
    assert(entropy > 1.58); // Should be close to log2(3) ≈ 1.585
    assert(calc.is_high_entropy());
    
    std::cout << "✓ Maximum entropy: " << entropy << " bits\n";
}

void test_window_size_adaptation() {
    std::cout << "Testing window size adaptation...\n";
    
    SlidingEntropyCalculator calc(10, 5, 20);
    assert(calc.get_window_size() == 0);
    
    // Add actions to trigger adaptation
    for (int i = 0; i < 15; ++i) {
        calc.add_action(static_cast<TraderAction>(i % 3));
    }
    
    size_t window_size = calc.get_window_size();
    assert(window_size >= 5 && window_size <= 20);
    
    std::cout << "✓ Window size adaptation: " << window_size << "\n";
}

void test_clear_and_reset() {
    std::cout << "Testing clear and reset...\n";
    
    SlidingEntropyCalculator calc(10);
    
    calc.add_action(TraderAction::BUY);
    calc.add_action(TraderAction::SELL);
    assert(calc.get_current_entropy() > 0.0);
    
    calc.clear();
    assert(calc.get_current_entropy() == 0.0);
    assert(calc.get_window_size() == 0);
    
    std::cout << "✓ Clear and reset passed\n";
}

void test_entropy_boundaries() {
    std::cout << "Testing entropy boundaries...\n";
    
    SlidingEntropyCalculator calc(100);
    
    // Test low entropy boundary
    for (int i = 0; i < 20; ++i) {
        calc.add_action(TraderAction::BUY);
    }
    assert(calc.is_low_entropy());
    
    calc.clear();
    
    // Test high entropy boundary
    for (int i = 0; i < 30; ++i) {
        calc.add_action(static_cast<TraderAction>(i % 3));
    }
    assert(calc.is_high_entropy());
    
    std::cout << "✓ Entropy boundaries passed\n";
}

int main() {
    std::cout << "Entropy Calculator Edge Case Tests\n";
    std::cout << "==================================\n\n";
    
    test_empty_window_entropy();
    test_single_action_type();
    test_maximum_entropy();
    test_window_size_adaptation();
    test_clear_and_reset();
    test_entropy_boundaries();
    
    std::cout << "\nAll entropy edge case tests passed!\n";
    return 0;
}
