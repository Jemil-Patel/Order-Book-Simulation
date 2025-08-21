#include "Utils.h"

namespace Utils {
Order generateRandomOrder(int id, std::mt19937& gen) {
    std::uniform_int_distribution<> typeDist(0, 1); // BUY or SELL
    std::uniform_real_distribution<> buyPriceDist(50.10, 50.20); 
    std::uniform_real_distribution<> sellPriceDist(49.80, 49.90);
    // std::uniform_real_distribution<> buyPriceDist(49.90, 50.20); 
    // std::uniform_real_distribution<> sellPriceDist(49.80, 50.10);

    std::uniform_int_distribution<> qtyDist(1, 100);   // 1-100 shares
    
    bool isBuy = typeDist(gen) == 0;
    double price = 0.0;
    
    if (isBuy) {
        price = std::round(buyPriceDist(gen) * 100) / 100;
    } else {
        price = std::round(sellPriceDist(gen) * 100) / 100;
    }

    int quantity = qtyDist(gen);
    
    return Order(id, isBuy, price, quantity);
}

double computeAvgLatency(const std::vector<std::chrono::microseconds>& latencies) {
    if (latencies.empty()) return 0.0;
    double sum = 0.0;
    for (const auto& d : latencies) sum += d.count();
    return sum / latencies.size();
}
}