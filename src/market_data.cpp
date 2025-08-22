#include "market_data.hpp"

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
