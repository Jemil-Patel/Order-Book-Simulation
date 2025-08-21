#pragma once
#include <string>
#include <chrono>

// Represents a single order in the book
struct Order {
    enum class Type { LIMIT }; // Extensible: add MARKET, ICEBERG, etc.
    int id;                    // Unique order ID
    Type type;                 // Order type
    bool isBuy;                // True for buy, false for sell
    double price;              // Limit price
    int quantity;              // Number of shares
    std::chrono::steady_clock::time_point timestamp; // For time-priority matching

    Order(int id, bool isBuy, double price, int quantity)
        : id(id), type(Type::LIMIT), isBuy(isBuy), price(price), quantity(quantity),
          timestamp(std::chrono::steady_clock::now()) {}
};