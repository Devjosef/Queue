# Queue Entropy Analysis

## Purpose & Goal

**Purpose**: Apply Shannon entropy from information theory to analyze trader behavior patterns and their relationship to market volatility using a concurrent pipeline.

**Goal**: Provide a tested, research implementation that computes Shannon entropy over trader actions, exercises the computation in simulated market scenarios, and exposes a pipeline for future benchmarking and real-data integration.

## Theory & Approach

### Shannon Entropy in Market Analysis
Shannon entropy quantifies the unpredictability of trader actions.

- **Low Entropy (0-0.5 bits)**: Predictable behavior (mass buying/selling)
- **Medium Entropy (0.5-1.2 bits)**: Mixed behavior patterns
- **High Entropy (1.2+ bits)**: Unpredictable, diverse behavior

### Core Hypothesis
Trader behavior entropy may correlate with market volatility. The repository provides simulation evidence that this relationship is nuanced and requires real-market validation.

## Methodology

### Data Collection
- **Trader Actions**: 0 (hold), 1 (buy), 2 (sell)
- **Time Window**: Adaptive sliding windows over sequential trading periods
- **Entropy Calculations**: H = -Σ(p_i * log2(p_i))
- **Concurrency**: Queue-based producer/consumer pipeline (mutex-based and hybrid implementations)

### Testing Framework
- **Unit Tests**: Validate entropy calculations with known distributions
- **Robustness Tests**: Edge cases (empty data, identical actions, random patterns)
- **Market Simulation**: Synthetic scenarios (Bull/Bear markets, crashes, recovery)
- **Performance Micro-benchmark**: Short synthetic HFT run used for extrapolated throughput measurements (see Performance notes)

## Implementation

### Core Entropy Calculation
```cpp
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
      entropy -= p * std::log2(p);
    }
  }
  return entropy;
}
```

### Concurrent Queue System
This repository contains two queue variants:

- `ConcurrentQueue` — a straightforward mutex-protected queue using `std::mutex` and `std::condition_variable`.
- `OptimizedQueue` — a hybrid design with separate head/tail mutexes, an atomic size counter, condition variables, batch pop support, and backpressure logic. It is optimized for throughput but is not a fully lock-free MPMC queue.

```cpp
template <typename T>
class ConcurrentQueue {
  void push(const T& value);
  bool try_pop(T& result);
  void wait_and_pop(T& result);
  bool empty() const;
private:
  mutable std::mutex m_mutex;
  std::queue<T> m_queue;
  std::condition_variable m_cond_var;
};
```

## Testing & Validation Results

### Mathematical Validation
- **Unit Tests**: Passed (entropy calculations match expectations for tested distributions)
- **Robustness Tests**: Edge cases handled (empty windows, single-action windows, equal distributions)
- **Mathematical Accuracy**: Matches theoretical expectations (max ~1.585 bits for 3-action equal distribution)

### Market Simulation Results
- **Status**: All provided synthetic simulations ran successfully (6/6 scenarios on this machine)
- **Representative findings**:
  - Crash scenarios show low entropy and heavy sell dominance in synthetic data.
  - Normal and bull/bear scenarios produce higher entropy consistent with mixed distributions.
  - The HFT simulation is a short micro-benchmark (5k events) that prints throughput/latency for that run; throughput is extrapolated and must be validated by dedicated benchmarking.

### Performance notes
- The repository contains a micro-benchmark within the market simulation that extrapolates throughput from a short (5k-event) run. That extrapolation can produce multi-million ops/sec figures on some machines, but this is a synthetic, short-duration measurement. It should not be cited as proof of sustained, production-grade 5M ops/sec throughput or guaranteed sub-millisecond latency without dedicated, reproducible benchmarking.

## Fixes applied during validation

- Fixed ordering in `include/market_pipeline.hpp` to increment `metrics_.total_processed` before computing average latency (prevents division-by-zero / NaN).
- Corrected a test assertion typo in `tests/test_market_simulation.cpp`.

## Validating the Thesis

The repository demonstrates that entropy can differentiate synthetic market conditions. However, the relationship between entropy and volatility is nuanced in simulations and requires real-market data and careful time-series analysis for confirmation.

## Conclusions

- The implementation provides a correct Shannon entropy calculation and a working, tested pipeline for research experiments.
- The queue implementations are mutex-based or hybrid (not lock-free).
- Simulations show promising patterns, but performance claims should be qualified as synthetic micro-benchmark results. Real-world validation is required.

## Usage

### Quick Start
```bash
make all
make test
make perf   # runs the market simulation micro-benchmark
```

### Manual Compilation
```bash
g++ -std=c++17 -I include -pthread -O3 -o market_entropy_analyzer src/*.cpp
```

## Files Structure
```
queue/
├── include/                    # Header files (queues, pipeline, entropy)
├── src/                        # Source files (entropy, market data, main)
├── tests/                      # Test suites and simulations
├── Makefile                    # Build and test targets
└── README.md                   # This file
```

## Technical Specifications

**Language**: C++17
**Queue Type**: Mutex-protected or hybrid (mutex + atomics). Not fully lock-free.
**Entropy Calculation**: Shannon entropy over sliding windows
**Memory Model**: Uses atomics for metrics; core queues use mutexes
**Thread Safety**: Thread-safe via mutexes and atomics
**Performance**: Micro-benchmark reports are synthetic; dedicated benchmarking required for sustained numbers
**Entropy Range**: 0.0 to ~1.585 bits (for 3 discrete actions)

## Dependencies

- C++17 or later
- pthread (for multithreading)

## References

- Shannon, C.E. (1948). "A Mathematical Theory of Communication"

## Notes

This project is research-oriented. The codebase and tests are useful for experiments and local validation. 
For comprehensive testing documentation, see `tests/README.md`



**Note**: All tests use simulated/mock data. Has yet to be tested on real market data.
