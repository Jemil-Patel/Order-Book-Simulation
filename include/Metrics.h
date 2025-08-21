// In Metrics.h
#include <vector>
#include <chrono>
#include <iostream>

namespace Metrics {

struct LatencyStats {
    double average = 0.0;
    long p50 = 0;
    long p90 = 0;
    long p99 = 0;
    long p999 = 0;
    long max = 0;
};

// Use nanoseconds
LatencyStats calculateLatencyStats(std::vector<std::chrono::nanoseconds>& latencies);

void printLatencyStats(std::ostream& os, const std::string& name, const LatencyStats& stats);

}