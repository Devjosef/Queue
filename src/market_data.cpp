// Data Container for Market Data Actions
#include "market_data.hpp"
#include <cstdlib>
#include <ctime>

// Add a trader action to the stored sequence
void MarketData::add_action(TraderAction action) {
    actions_.push_back(action);
}

// Retrieve all recorded trader actions
const std::vector<TraderAction>& MarketData::get_actions() const {
    return actions_;
}

// Clear all stored trader actions
void MarketData::clear() {
    actions_.clear();
}

double get_spy_price() {
    static double price = 695.42;
    price += (rand() % 40 - 20) * 0.001;  // ±0.02% random walk
    return price;
}

TraderAction get_spy_action(double price_change_pct) {
    return (price_change_pct > 0.05) ? TraderAction::BUY : 
           (price_change_pct < -0.05) ? TraderAction::SELL : TraderAction::HOLD;
}