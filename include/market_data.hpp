#ifndef MARKET_DATA_HPP
#define MARKET_DATA_HPP

#include <vector>
#include <cstdint> // specifically added for fixed-width integer types

enum class TraderAction : uint8_t {
    HOLD = 0,
    BUY = 1,
    SELL = 2
};

class MarketData {
    public:
        MarketData() = default;
         ~MarketData() = default;

         MarketData(const MarketData&) = default;
         MarketData& operator=(const MarketData&) = default;

         void add_action(TraderAction action);

         const std::vector<TraderAction>& get_actions() const;

         void clear();

    private:
        std::vector<TraderAction> actions_;
};

#endif // MARKET_DATA_HPP