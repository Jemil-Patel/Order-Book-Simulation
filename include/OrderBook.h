#pragma once
#include "Order.h"
#include <map>
#include <list>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <iostream>
#include <chrono>

class OrderBook {
public:
    OrderBook();
    
    void startMarket();

    // Add order to book, returns true if added successfully
    bool addOrder(const Order& order);

    // Cancel order by ID, returns true if canceled
    bool cancelOrder(int orderId);

    // Match orders, returns number of trades executed in this call
    int matchOrders();

    // Display current book state (top 5 levels) and metrics
    void display(std::ostream& os = std::cout) const;

    // Get average add/match latencies (in microseconds)
    double getAvgAddLatency() const;
    double getAvgMatchLatency() const;

    // Get latency vectors for logging/plotting
    const std::vector<std::chrono::microseconds>& getAddLatencies() const { return addLatencies; }
    const std::vector<std::chrono::microseconds>& getMatchLatencies() const { return matchLatencies; }
    
    mutable std::mutex mutex; // Protects book
    std::condition_variable cv; // Signals matcher

    bool newOrderAdded = false;
private:
    struct PriceLevel {
        int totalQuantity = 0;
        std::list<Order> orders; // FIFO time priority (earliest first)
    };

    struct OrderLocation {
        double price;
        std::list<Order>::iterator iter;
        bool isBuy;
    };

    // Bids: sorted descending by price
    std::map<double, PriceLevel, std::greater<double>> bidLevels;
    // Asks: sorted ascending by price
    std::map<double, PriceLevel> askLevels;

    std::unordered_map<int, OrderLocation> orderLocations; // For fast cancels


    std::vector<std::chrono::microseconds> addLatencies; // Tracks add times
    std::vector<std::chrono::microseconds> matchLatencies; // Tracks match times

    std::string lastTrade; // Stores last trade for display
    int totalOrdersAdded = 0; // Counter for all added orders
    int totalTradesExecuted = 0; // Counter for all executed trades

    std::chrono::steady_clock::time_point startTime; // For throughput metrics
};