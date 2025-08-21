#include "OrderBook.h"
#include <sstream>
#include <iomanip>
#include <cstdlib> // For system("clear")

OrderBook::OrderBook() : startTime(std::chrono::steady_clock::now()) {}

// In OrderBook.cpp
void OrderBook::startMarket() {
    // Reset counters and timer for the start of concurrent trading
    addLatencies.clear();
    matchLatencies.clear();
    startTime = std::chrono::steady_clock::now();
}

bool OrderBook::addOrder(const Order& order) {
    auto start = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> lock(mutex);

    if (order.quantity <= 0 || order.price <= 0) return false;

    if (order.isBuy) {
        auto& level = bidLevels[order.price];
        level.orders.push_back(order);
        level.totalQuantity += order.quantity;
        orderLocations[order.id] = {order.price, --level.orders.end(), order.isBuy};
    } else {
        auto& level = askLevels[order.price];
        level.orders.push_back(order);
        level.totalQuantity += order.quantity;
        orderLocations[order.id] = {order.price, --level.orders.end(), order.isBuy};
    }

    totalOrdersAdded++;

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start);
    addLatencies.push_back(duration);

    newOrderAdded = true;
    cv.notify_one();
    return true;
}

bool OrderBook::cancelOrder(int orderId) {
    std::lock_guard<std::mutex> lock(mutex);

    auto it = orderLocations.find(orderId);
    if (it == orderLocations.end()) return false;

    const auto& loc = it->second;

    if (loc.isBuy) {
        auto levelIt = bidLevels.find(loc.price);
        if (levelIt == bidLevels.end()) return false;

        auto& level = levelIt->second;
        level.totalQuantity -= loc.iter->quantity;
        level.orders.erase(loc.iter);

        if (level.orders.empty()) {
            bidLevels.erase(levelIt);
        }
    } else {
        auto levelIt = askLevels.find(loc.price);
        if (levelIt == askLevels.end()) return false;
        
        auto& level = levelIt->second;
        level.totalQuantity -= loc.iter->quantity;
        level.orders.erase(loc.iter);

        if (level.orders.empty()) {
            askLevels.erase(levelIt);
        }
    }

    orderLocations.erase(it);
    return true;
}

int OrderBook::matchOrders() {
    auto start = std::chrono::steady_clock::now();
    // std::lock_guard<std::mutex> lock(mutex); // Use guard since no wait inside

    int trades = 0;
    while (!bidLevels.empty() && !askLevels.empty()) {
        auto& bestBidLevel = bidLevels.begin()->second;
        auto& bestAskLevel = askLevels.begin()->second;
        double bestBidPrice = bidLevels.begin()->first;
        double bestAskPrice = askLevels.begin()->first;

        if (bestBidPrice < bestAskPrice) break;

        Order& bidOrder = bestBidLevel.orders.front();
        Order& askOrder = bestAskLevel.orders.front();

        int tradeQty = std::min(bidOrder.quantity, askOrder.quantity);

        // More realistic trade price: use the price of the order that was there first.
        double tradePrice = (bidOrder.timestamp < askOrder.timestamp) ? bestBidPrice : bestAskPrice;

        // Record trade
        std::stringstream ss;
        ss << "Traded " << tradeQty << " shares @ $" << std::fixed << std::setprecision(2) << tradePrice;
        lastTrade = ss.str();

        // Update quantities
        bidOrder.quantity -= tradeQty;
        askOrder.quantity -= tradeQty;
        bestBidLevel.totalQuantity -= tradeQty;
        bestAskLevel.totalQuantity -= tradeQty;

        // Remove if fully filled
        if (bidOrder.quantity == 0) {
            orderLocations.erase(bidOrder.id);
            bestBidLevel.orders.pop_front();
        }
        if (askOrder.quantity == 0) {
            orderLocations.erase(askOrder.id);
            bestAskLevel.orders.pop_front();
        }

        // Erase empty levels
        if (bestBidLevel.orders.empty()) {
            bidLevels.erase(bidLevels.begin());
        }
        if (bestAskLevel.orders.empty()) {
            askLevels.erase(askLevels.begin());
        }

        trades++;
    }

    totalTradesExecuted += trades;

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start);
    matchLatencies.push_back(duration);

    return trades;
}

void OrderBook::display(std::ostream& os) const {
    std::lock_guard<std::mutex> lock(mutex);

    // Clear console for macOS
    system("clear");

    os << "===== Order Book =====\n";
    os << "Bids:\n";
    int count = 0;
    for (const auto& [price, level] : bidLevels) {
        if (count++ >= 5) break;
        os << "$" << std::fixed << std::setprecision(2) << price << " | " << level.totalQuantity << "\n";
    }
    os << "Asks:\n";
    count = 0;
    for (const auto& [price, level] : askLevels) {
        if (count++ >= 5) break;
        os << "$" << std::fixed << std::setprecision(2) << price << " | " << level.totalQuantity << "\n";
    }
    os << "Last Trade: " << (lastTrade.empty() ? "None" : lastTrade) << "\n";
    os << "Total Orders Added: " << totalOrdersAdded << "\n";
    os << "Total Trades Executed: " << totalTradesExecuted << "\n";

    // Throughput metrics
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(now - startTime).count();
    if (elapsed > 0) {
        os << "Orders/sec: " << std::fixed << std::setprecision(2) << (totalOrdersAdded / elapsed) << "\n";
        os << "Trades/sec: " << std::fixed << std::setprecision(2) << (totalTradesExecuted / elapsed) << "\n";
    }

    os << "Avg Add Latency: " << getAvgAddLatency() << " us\n";
    os << "Avg Match Latency: " << getAvgMatchLatency() << " us\n";
    os << "==================\n";
}

double OrderBook::getAvgAddLatency() const {
    if (addLatencies.empty()) return 0.0;
    double sum = 0.0;
    for (const auto& d : addLatencies) sum += d.count();
    return sum / addLatencies.size();
}

double OrderBook::getAvgMatchLatency() const {
    if (matchLatencies.empty()) return 0.0;
    double sum = 0.0;
    for (const auto& d : matchLatencies) sum += d.count();
    return sum / matchLatencies.size();
}