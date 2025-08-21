#include "Utils.h"
#include <iostream>

namespace Utils {
Order generateRandomOrder(int id, std::mt19937& gen) {
    std::uniform_int_distribution<> typeDist(0, 1); // BUY or SELL
    std::uniform_real_distribution<> buyPriceDist(50.10, 50.20); 
    std::uniform_real_distribution<> sellPriceDist(49.80, 49.90);
    // std::cout << "heresdijfwuhgfmwiouhfwoiuhfxmwuhfwmiuefh";
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

Order generateRealisticOrder(int id, std::mt19937& gen, double& midPrice, double volatility, double spread) {
    // 1. Simulate a gentle, mean-reverting price drift for the mid-price.
    const double centralPrice = 2500.0; // A higher central price for a wider book
    const double reversionFactor = 0.001;
    std::normal_distribution<> drift_dist(0.0, volatility);
    midPrice += drift_dist(gen) + reversionFactor * (centralPrice - midPrice);

    // 2. Define the two-sided market ranges. This is the critical part.
    // Bids (buy orders) will be generated BELOW the mid-price.
    // Asks (sell orders) will be generated ABOVE the mid-price.
    const double priceRange = 500.0; // The depth of each side of the book
    double bid_side_top = midPrice - (spread / 2.0);
    double ask_side_bottom = midPrice + (spread / 2.0);
    
    std::uniform_real_distribution<> bid_price_dist(bid_side_top - priceRange, bid_side_top);
    std::uniform_real_distribution<> ask_price_dist(ask_side_bottom, ask_side_bottom + priceRange);

    // 3. Decide the side first. The price will depend on the side.
    std::uniform_int_distribution<> side_dist(0, 1);
    bool isBuy = side_dist(gen) == 0;
    
    double finalPrice;
    if (isBuy) {
        finalPrice = bid_price_dist(gen);
    } else { // isSell
        finalPrice = ask_price_dist(gen);
    }
    
    // 4. Introduce occasional "aggressive" orders that cross the spread.
    std::uniform_real_distribution<> aggressive_dist(0.0, 1.0);
    if (aggressive_dist(gen) < 0.05) { // 5% of orders will be aggressive
        if (isBuy) {
            // Aggressive buy takes a price from the ask side
            finalPrice = ask_price_dist(gen);

        } else {
            // Aggressive sell takes a price from the bid side
            finalPrice = bid_price_dist(gen);
        }
    }

    finalPrice = std::round(finalPrice * 100.0) / 100.0;
    if (finalPrice <= 0) finalPrice = 0.01;

    // 5. Generate quantity
    std::uniform_int_distribution<> qty_dist(5, 100);
    int quantity = qty_dist(gen);

    return Order(id, isBuy, finalPrice, quantity);
}
}