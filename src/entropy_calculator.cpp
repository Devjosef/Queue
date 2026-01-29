// Core algorithm to compute Shannon entropy for TraderAction sequences
#include "entropy_calculator.hpp"
#include <cmath>
#include <map>

// Calculate Shannon entropy of actions; returns 0 if empty
double EntropyCalculator::calculate_entropy(const std::vector<TraderAction>& actions) {
    if (actions.empty()) return 0.0;

    std::map<TraderAction, int> counts;
    for (const auto& action : actions) {
        counts[action]++;
    }

    double entropy = 0.0;
    int total = static_cast<int>(actions.size());

    for (const auto& [action, count] : counts) {
        double p = static_cast<double>(count) / total;
        if (p > 0) {
            entropy -= p * std::log2(p);  // Entropy contribution
        }
    }

    return entropy;
}
