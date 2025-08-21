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
