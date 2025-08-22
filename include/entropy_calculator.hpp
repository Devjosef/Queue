#ifndef ENTROPY_CALCULATOR_HPP
#define ENTROPY_CALCULATOR_HPP

#include <vector>
#include "market_data.hpp"

class EntropyCalculator {
public:
    EntropyCalculator() = default;
    ~EntropyCalculator() = default;
    
    EntropyCalculator(const EntropyCalculator&) = delete;
    EntropyCalculator& operator=(const EntropyCalculator&) = delete;
    
    double calculate_entropy(const std::vector<TraderAction>& actions);
    
    static double get_max_entropy() { return 1.585; }
    static bool is_high_entropy(double entropy) { return entropy > 1.2; }
    static bool is_low_entropy(double entropy) { return entropy < 0.5; }
    static bool is_medium_entropy(double entropy) { 
        return entropy >= 0.5 && entropy <= 1.2; 
    }
};

#endif // ENTROPY_CALCULATOR_HPP
