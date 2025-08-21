Limit Order Book Simulator
A C++-based simulator for a financial exchange's limit order book, demonstrating low-latency order processing, multi-threading, and real-time visualization.
Features

Random Order Generation: Generates BUY/SELL limit orders every t milliseconds (configurable).
Pre-loading: Populates book with N random orders before market opens.
Threading: Three threads for generating orders, matching trades, and displaying the book.
Visualization: Console-based table of top 5 bid/ask levels, last trade, and average add/match latencies (in microseconds).
Extensibility: Order type enum supports adding new types (e.g., market orders).
Performance: Tracks and displays average latencies for order adds and matches.
Priority Queue Processing: Uses map for sorted price levels and lists for FIFO orders per level, optimizing for low latency (O(log L) inserts where L=price levels).
Throughput Metrics: Orders/sec and trades/sec displayed in real-time.
Latency Plotting: Logs latencies to CSV; use provided Python script to plot.

Build Instructions
mkdir build && cd build
cmake ..
make

Usage
Run with optional arguments:

Number of pre-loaded orders (default: 100)
Order generation interval in ms (default: 100)
Simulation duration in seconds (default: 60)

Example:
./simulator 200 50 30


Pre-loads 200 orders, generates new orders every 50ms, runs for 30s.

Plotting Latencies
After running, use the provided plot_latencies.py (requires Python with matplotlib):
python plot_latencies.py


Generates "latencies.png" showing add/match latencies over operations.

Output
Console shows:

Initial "Market opened" message.
Periodic order book updates (every 500ms): top 5 bids/asks, last trade, totals, throughput, avg latencies.
Final metrics and CSV logs at end.

Future Extensions

Add market orders or iceberg orders via Order::Type.
Support file-based order input (e.g., parse "BUY 100 shares @ $50").
Implement risk checks (e.g., balance limits).
Log trades to file for analysis.

//

# High-Performance C++ Order Book Simulation

This project is a multi-threaded simulation of a limit order book (LOB), designed to model the core functionality of a financial exchange's matching engine. It is written in modern C++ and focuses on performance, correctness, and realistic simulation of market dynamics.

The primary goal is to provide a framework for measuring and analyzing the latency and throughput of a matching engine under various conditions.

## Key Features

*   **Multi-Threaded Architecture**: Utilizes separate threads for order generation, matching, and UI display to simulate a concurrent environment.
*   **Price-Time Priority Matching**: Correctly implements the standard algorithm for matching orders: best price first, then first-come-first-served (FIFO).
*   **Realistic Order Generation**: Simulates a dynamic market with a stateful generator that models price drift, spread, and a mix of passive (liquidity-providing) and aggressive (liquidity-taking) orders.
*   **Detailed Performance Metrics**:
    *   **Throughput**: Measures total Orders per Second (OPS) and Trades per Second (TPS).
    *   **Latency Distribution**: Calculates average, median (p50), p90, p99, p99.9, and max latencies for critical operations (adding an order, executing a match). This is crucial for understanding tail latency behavior.
*   **Data Visualization**: Outputs latency data to CSV files, which can be visualized using the provided Python script (`plot_latencies.py`) to analyze performance over time.

## Project Structure

```
.
├── Order.h/cpp          # Defines the basic Order struct.
├── OrderBook.h/cpp      # The core matching engine logic and data structures.
├── Utils.h/cpp          # Helper functions for generating realistic random orders.
├── Metrics.h/cpp        # Utilities for calculating detailed latency statistics (p50, p99, etc.).
├── main.cpp             # Main driver: sets up threads, runs the simulation, and reports final metrics.
├── plot_latencies.py    # Python script to visualize latency data from CSV output.
└── README.md            # This file.
```

## Core Design and Data Structures

The heart of the project is the `OrderBook` class.

*   **Data Structures**:
    *   **Bids**: `std::map<double, PriceLevel, std::greater<double>>`
        *   A map sorted by price in descending order (highest bid first).
    *   **Asks**: `std::map<double, PriceLevel>`
        *   A map sorted by price in ascending order (lowest ask first).
    *   **PriceLevel**: `struct` containing a `std::list<Order>` to maintain strict FIFO order for time priority.
    *   **Order Location Cache**: `std::unordered_map<int, OrderLocation>`
        *   Provides O(1) average-case lookup for fast order cancellations by mapping an `orderId` to an iterator in the corresponding price level's list.

*   **Concurrency**:
    *   A global `std::mutex` protects the order book data structures from concurrent access.
    *   A `std::condition_variable` is used to signal the matcher thread efficiently, ensuring it only wakes up when a new order has been added, avoiding busy-waiting.

## Approach to Realism

Several steps were taken to ensure the simulation provides meaningful, near-realistic performance data:

1.  **Stateful Order Generation**: Instead of stateless random orders, the generator maintains a `midPrice` that follows a random walk. New orders are generated relative to this price, creating a realistic clustering of orders around the bid-ask spread.
2.  **Order Type Mixture**: The simulation generates both passive "maker" orders that build the book and aggressive "taker" orders that cross the spread and create trades, mimicking real market participant behavior.
3.  **Granular Latency Measurement**: Match latency is only recorded when one or more trades are actually executed, preventing the average from being skewed by thousands of "no-op" matching attempts. The measurement is then normalized per trade to provide a more accurate latency figure.

## Performance Analysis

The simulation reports key performance indicators upon completion:

*   **Throughput (OPS/TPS)**: Measures how many orders and trades the engine can process per second. This is calculated based on the active simulation time after the initial book setup.
*   **Latency Profile**:
    *   **Add Latency**: The time taken to insert a new order into the book. Dominated by the `std::map` lookup and insertion time, which is logarithmic, O(log N), where N is the number of price levels.
    *   **Match Latency**: The time taken to execute a batch of trades when the book is crossed. This metric is critical as it represents the core work of the engine.
    *   **Tail Latency (p99, p99.9)**: These metrics are vital for high-frequency trading systems, as they reveal the worst-case performance that can be expected under load. High tail latency can indicate issues with memory allocation, cache misses, or thread contention.

### Future Work & Scaling Considerations

*   **Scaling Tests**: To rigorously test the `O(log N)` complexity, the simulation could be run with varying pre-load depths (e.g., 10k, 100k, 1M orders) to graph how latency scales.
*   **Architectural Improvement**: The current global mutex is a known bottleneck. A production-grade system would replace this with a single-threaded matching core that processes orders from a lock-free queue, eliminating lock contention on the critical path entirely.
*   **Data Structure Optimization**: For ultimate performance, the `std::map` could be replaced with a `std::vector` indexed by integer price ticks, changing price level lookups from `O(log N)` to `O(1)` and significantly improving cache locality.

Rigorous Performance Measurement
The simulation includes a benchmark mode designed for rigorous performance analysis, addressing common pitfalls in latency measurement:

Unthrottled Throughput Testing: In benchmark mode (./simulator ... benchmark), the order generator runs at maximum speed, revealing the true Orders Per Second (OPS) the engine can sustain. This generates millions of events required for statistically stable tail-latency analysis.
Elimination of Measurement Noise: Latency timers are started after locks are acquired. This ensures that measurements reflect only the engine's processing time, excluding external factors like I/O blocking or thread scheduling waits, which previously caused a pathologically large latency tail.
Realistic Workload Generation: The order generator has been enhanced to model a mean-reverting price walk and generate a realistic mix of small, passive "maker" orders and larger, aggressive "taker" orders. This creates a balanced and dynamic order book, preventing the artificial skew seen in simpler models and providing a more challenging and realistic test for the matching engine.

Understanding Tail Latency
In benchmark mode, the engine can process millions of orders per second. The latency profile shows a median (p50) add time in the hundreds of nanoseconds, which is consistent with the expected performance of std::map on modern hardware.

However, a significant tail latency is observed (e.g., p99.9 and max values can be orders of magnitude higher). This is a well-understood artifact of the underlying data structures:

Cause: The extreme outliers are caused by heap allocations when std::map needs to create a new price level for the first time. This involves a call to the system's memory allocator (malloc/new), which can trigger expensive operations like requesting memory pages from the OS kernel, leading to a multi-millisecond stall.
Implication: While most operations are fast (inserting into an existing price level), the cost of creating new levels dominates the tail. A production system would mitigate this by using custom memory allocators (pool allocators) or by pre-allocating a fixed-size data structure (like a std::vector indexed by price ticks) to eliminate dynamic allocations on the critical path entirely.

## How to Build and Run

1.  **Build**:
    ```bash
    mkdir build && cd build
    cmake ..
    make
    ```
2.  **Run**:
    ```bash
    ./OrderBookSim [pre_orders] [interval_ms] [duration_s]
    # Example: 1000 pre-orders, new order every 10ms, run for 30s
    ./OrderBookSim 1000 10 30
    ```
3.  **Visualize**:
    ```bash
    python3 ../plot_latencies.py
    ```