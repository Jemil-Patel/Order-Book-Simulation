# C++ Order Book Simulation

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

## How to Build and Run

### Prerequisites
*   A C++17 compliant compiler (`clang++` or `g++`)
*   **CMake** (version 3.10+)
*   **Make**
*   **Python 3** with `matplotlib` (optional, for plotting latency charts)

### 1. Build

From the repository root directory:

```bash
# Create and navigate to the build directory
mkdir -p build && cd build

# Configure with CMake
cmake ..

# Build the executable
make
```

This will produce the executable binary named **`simulator`** inside the `build/` directory.

### 2. Run

You can run the simulator directly from the root directory or from inside the `build/` directory.

**From project root:**
```bash
./build/simulator [pre_orders] [duration_s]
```

**From the `build/` directory:**
```bash
./simulator [pre_orders] [duration_s]
```

#### CLI Parameters Explained

The simulator accepts optional positional arguments:

| Parameter | Type | Default | Description |
| :--- | :--- | :--- | :--- |
| `pre_orders` | Integer | `10000` | **Initial Order Book Depth**: The number of realistic random orders pre-loaded and matched *before* opening the market. This populates both bid and ask queues around the mid-price so the simulation starts with realistic liquidity. |
| `duration_s` | Integer | `30` | **Simulation Duration**: The number of seconds the real-time order generation and matching engine thread will run before shutting down and printing throughput and latency percentiles. |

#### Run Command Examples

*   **Run with Defaults** (10,000 pre-loaded orders, runs for 30 seconds):
    ```bash
    ./build/simulator
    ```

*   **Quick Test Run** (1,000 pre-loaded orders, runs for 5 seconds):
    ```bash
    ./build/simulator 1000 5
    ```

*   **Heavy Load Benchmark** (50,000 pre-loaded orders, runs for 60 seconds):
    ```bash
    ./build/simulator 50000 60
    ```

*   **Legacy 3-Argument Syntax**:
    ```bash
    ./build/simulator 10000 0 10
    ```

#### Sample Output

When the run finishes, the engine outputs throughput metrics and nanosecond-level latency percentiles:

```text
Market opened with 1000 pre-loaded orders
--- RUNNING IN SINGLE-THREADED BENCHMARK MODE ---
Generator will run in real-time, and a single thread will process for 5 seconds.
Simulation ended. Final book state shown above.

--- Throughput (Engine-Only Active Time) ---
Total Orders Processed:   3982410
Total Trades Executed:    387120
Active Engine Time:       2.314201 s (Wall-clock: 5.002130 s)
Orders per Second (OPS):  1720857.44
Trades per Second (TPS):  167280.20
--------------------------------------------

--- Add Order Latency (ns) ---
Average: 489.12
p50 (Median): 417
p90: 625
p99: 1375
p99.9: 4500
Max: 512083
-------------------------
--- Match Operation Latency (ns) ---
Average: 1084.50
p50 (Median): 958
p90: 1083
p99: 1750
p99.9: 7250
Max: 498333
-------------------------
```

### 3. Visualize Latency (Optional)

You can generate graphical latency plots over time:

1. In [`src/main.cpp`](src/main.cpp), uncomment lines 100–111 to enable writing latencies to `add_latencies.csv` and `match_latencies.csv`.
2. Recompile and run the simulator:
   ```bash
   make -C build
   ./build/simulator 10000 5
   ```
3. Run the visualization script from the project root (using the provided virtual environment or any Python environment with `matplotlib`):
   ```bash
   source venv/bin/activate  # or: pip install matplotlib
   python3 plot_latencies.py
   ```
   This will read the CSV files and generate **`latencies.png`**.

## Project Structure

```
.
├── CMakeLists.txt       # CMake build configuration.
├── include/             # C++ header files
│   ├── Metrics.h        # Percentiles (p50, p90, p99, p99.9) and stats utilities.
│   ├── Order.h          # Basic Order struct definition.
│   ├── OrderBook.h      # Core matching engine and book data structures.
│   └── Utils.h          # Helper functions for realistic random order generation.
├── src/                 # C++ source files
│   ├── Metrics.cpp
│   ├── Order.cpp
│   ├── OrderBook.cpp
│   ├── Utils.cpp
│   └── main.cpp         # Main benchmark driver and simulation orchestrator.
├── plot_latencies.py    # Python script to visualize latency data from CSV output.
├── theory/              # In-depth architectural & theoretical documentation.
└── README.md            # Project documentation and usage guide.
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

### Rigorous Performance Measurement

The simulation includes a benchmark mode designed for rigorous performance analysis, addressing common pitfalls in latency measurement:

*   **Unthrottled Throughput Testing**: In benchmark mode, the order generator runs at maximum speed in real-time, revealing the true Orders Per Second (OPS) the engine can sustain. This generates millions of events required for statistically stable tail-latency analysis.
*   **Elimination of Measurement Noise**: Latency timers measure active matching engine operations directly, excluding external factors like I/O blocking or thread scheduling waits.
*   **Realistic Workload Generation**: The order generator has been enhanced to model a mean-reverting price walk and generate a realistic mix of small, passive "maker" orders and larger, aggressive "taker" orders. This creates a balanced and dynamic order book, providing a challenging and realistic test for the matching engine.

### Understanding Tail Latency

In benchmark mode, the engine can process millions of orders per second. The latency profile shows a median (p50) add time in the hundreds of nanoseconds, which is consistent with the expected performance of `std::map` on modern hardware.

However, a significant tail latency is observed (e.g., p99.9 and max values can be orders of magnitude higher). This is a well-understood artifact of the underlying data structures:

*   **Cause**: The extreme outliers are caused by heap allocations when `std::map` needs to create a new price level for the first time. This involves a call to the system's memory allocator (`malloc`/`new`), which can trigger expensive operations like requesting memory pages from the OS kernel, leading to a multi-millisecond stall.
*   **Implication**: While most operations are fast (inserting into an existing price level), the cost of creating new levels dominates the tail. A production system would mitigate this by using custom memory allocators (pool allocators) or by pre-allocating a fixed-size data structure (like a `std::vector` indexed by price ticks) to eliminate dynamic allocations on the critical path entirely.
