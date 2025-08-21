#pragma once
#include "Order.h"
#include <random>
#include <chrono>

namespace Utils {
// Generate a random limit order (BUY/SELL, price in [45,55], qty in [1,100])
Order generateRandomOrder(int id, std::mt19937& gen);

Order generateRealisticOrder(int id, std::mt19937& gen, double& midPrice, double volatility, double spread);

// Compute average duration from a vector of durations
double computeAvgLatency(const std::vector<std::chrono::microseconds>& latencies);
}