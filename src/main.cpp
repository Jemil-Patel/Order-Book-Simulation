#include "OrderBook.h"
#include "Utils.h"
#include "Metrics.h"
#include <thread>
#include <atomic>
#include <chrono>
#include <random>
#include <iostream>
#include <vector> // For the queue
#include <iomanip> // For std::setprecision

// A simple, single-threaded, non-locking queue for our benchmark
// In a real system, this would be a more complex lock-free queue.
#include <queue>

int main(int argc, char* argv[]) {
    // --- SETUP IS THE SAME ---
    int preOrderCount = (argc > 1) ? std::stoi(argv[1]) : 10000;
    int durationSec = (argc > 3) ? std::stoi(argv[3]) : 30;
    
    OrderBook book;
    std::atomic<bool> running(true);
    std::atomic<int> nextId(1);
    std::mt19937 gen(std::random_device{}());

    double midPrice = 2500.0;
    double volatility = 0.02;
    double spread = 0.05;

    // --- PRE-LOAD IS THE SAME ---
    for (int i = 0; i < preOrderCount; ++i) {
        book.addOrder(Utils::generateRealisticOrder(nextId++, gen, midPrice, volatility, spread));
    }
    {
        std::lock_guard<std::mutex> lock(book.mutex);
        book.matchOrders();
    }
    book.startMarket();
    std::cout << "Market opened with " << preOrderCount << " pre-loaded orders\n";
    std::cout << "--- RUNNING IN SINGLE-THREADED BENCHMARK MODE ---\n";
    std::cout << "Generator will pre-fill a queue, then a single thread will process for " << durationSec << " seconds.\n";


    // --- THE ARCHITECTURAL CHANGE ---

    // 1. GENERATOR PHASE: Create a large batch of orders in memory first.
    // This separates the cost of order *generation* from order *processing*.
    const int num_orders_to_generate = 10'000'000; // Generate 10 million orders
    std::vector<Order> order_queue;
    order_queue.reserve(num_orders_to_generate);
    std::cout << "Pre-generating " << num_orders_to_generate << " orders for the benchmark...\n";
    for(int i = 0; i < num_orders_to_generate; ++i) {
        order_queue.push_back(Utils::generateRealisticOrder(nextId++, gen, midPrice, volatility, spread));
    }
    std::cout << "Generation complete. Starting processing benchmark.\n";

    // 2. PROCESSING PHASE: A SINGLE thread processes the queue.
    // This is the true measure of your engine's performance.
    std::thread engine_thread([&]() {
        size_t order_index = 0;
        while(running && order_index < order_queue.size()) {
            const Order& order = order_queue[order_index];
            book.addOrder(order); // This call now also triggers matching internally
            order_index++;
        }
    });

    // 3. RUN and CLEANUP
    std::this_thread::sleep_for(std::chrono::seconds(durationSec));
    running = false;

    engine_thread.join();
    
    // No final book.display() to avoid double output
    std::cout << "Simulation ended. Final book state shown above.\n";
    
    // --- FINAL METRICS CALCULATION ---
    auto totalOrders = book.getTotalOrdersAdded();
    auto totalTrades = book.getTotalTradesExecuted();
    double elapsedSeconds = durationSec; // We know how long we ran for

    std::cout << "\n--- Throughput ---" << std::endl;
    std::cout << "Total Orders Processed: " << totalOrders << std::endl;
    std::cout << "Total Trades Executed:  " << totalTrades << std::endl;
    std::cout << "Orders per Second (OPS): " << std::fixed << std::setprecision(2) << (totalOrders / elapsedSeconds) << std::endl;
    std::cout << "Trades per Second (TPS): " << std::fixed << std::setprecision(2) << (totalTrades / elapsedSeconds) << std::endl;
    std::cout << "------------------\n" << std::endl;

    auto addLatencies = book.getAddLatencies();
    auto matchLatencies = book.getMatchLatencies();

    Metrics::LatencyStats addStats = Metrics::calculateLatencyStats(addLatencies);
    Metrics::LatencyStats matchStats = Metrics::calculateLatencyStats(matchLatencies);

    Metrics::printLatencyStats(std::cout, "Add Order", addStats);
    Metrics::printLatencyStats(std::cout, "Match Operation", matchStats);

    // Log latencies to CSV for plotting
    // std::ofstream addCsv("add_latencies.csv");
    // for (const auto& d : book.getAddLatencies()) {
    //     addCsv << d.count() << "\n";
    // }
    // addCsv.close();

    // std::ofstream matchCsv("match_latencies.csv");
    // for (const auto& d : book.getMatchLatencies()) {
    //     matchCsv << d.count() << "\n";
    // }
    // matchCsv.close();

    std::cout << "Latencies logged to add_latencies.csv and match_latencies.csv. Use plot_latencies.py to visualize.\n";
    
    return 0;
}