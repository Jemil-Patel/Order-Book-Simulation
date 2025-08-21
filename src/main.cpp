#include "OrderBook.h"
#include "Utils.h"
#include <thread>
#include <atomic>
#include <chrono>
#include <random>
#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
    // Parse args: pre-orders count, interval (ms), duration (s)
    int preOrderCount = (argc > 1) ? std::stoi(argv[1]) : 100;
    int intervalMs = (argc > 2) ? std::stoi(argv[2]) : 100;
    int durationSec = (argc > 3) ? std::stoi(argv[3]) : 60;
    
    OrderBook book;
    std::atomic<bool> running(true);
    std::atomic<int> nextId(1);
    std::mt19937 gen(std::random_device{}());
    
    for (int i = 0; i < preOrderCount; ++i) {
        book.addOrder(Utils::generateRandomOrder(nextId++, gen));
    }
    // Lock the book before the initial match
    {
        std::lock_guard<std::mutex> lock(book.mutex);
        book.matchOrders(); // One final match on the pre-loaded book
    }
    
    std::cout << "Market opened with " << preOrderCount << " pre-loaded orders\n";
    book.startMarket(); // clock starts
    
    // Generator thread: Add random orders every intervalMs
    std::thread generator([&]() {
        while (running) {
            book.addOrder(Utils::generateRandomOrder(nextId++, gen));
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
        }
    });
    
    // Matcher thread: Match orders on notification
    std::thread matcher([&]() {
        while (running) {
            std::unique_lock<std::mutex> lock(book.mutex);
            
            // 1. Wait until there is work to do.
            book.cv.wait(lock, [&]() { return !running || book.newOrderAdded; });
            
            // 2. If we are just stopping, exit the loop.
            if (!running) break;
            
            // 3. We have work. Reset the flag now that we've acknowledged the signal.
            book.newOrderAdded = false;
            
            // 4. Perform the matching. The lock is still held by 'lock'.
            book.matchOrders();
            
            // 5. The lock is automatically released here when unique_lock goes out of scope.
        }
    });
    
    // Display thread: Show book and latencies every 500ms
    std::thread displayer([&]() {
        while (running) {
            book.display();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });
    
    // Run for duration, then stop
    std::this_thread::sleep_for(std::chrono::seconds(durationSec));
    running = false;
    
    // Signal matcher to wake and exit
    book.cv.notify_one();
    
    // Cleanup
    generator.join();
    matcher.join();
    displayer.join();
    
    // No final book.display() to avoid double output
    std::cout << "Simulation ended. Final book state shown above.\n";
    
    // Log latencies to CSV for plotting
    std::ofstream addCsv("add_latencies.csv");
    for (const auto& d : book.getAddLatencies()) {
        addCsv << d.count() << "\n";
    }
    addCsv.close();

    std::ofstream matchCsv("match_latencies.csv");
    for (const auto& d : book.getMatchLatencies()) {
        matchCsv << d.count() << "\n";
    }
    matchCsv.close();

    std::cout << "Latencies logged to add_latencies.csv and match_latencies.csv. Use plot_latencies.py to visualize.\n";
    
    return 0;
}