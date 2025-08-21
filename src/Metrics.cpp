#include "Metrics.h"
#include <algorithm>
#include <cmath>
#include <iomanip>

namespace Metrics {

LatencyStats calculateLatencyStats(std::vector<std::chrono::nanoseconds>& latencies) {
    LatencyStats stats;
    if (latencies.empty()) {
        return stats;
    }

    // Sort the vector to calculate percentiles.
    // We pass by value or sort a copy if the original order is needed elsewhere.
    // For this project, sorting in-place is fine as we do it once at the end.
    std::sort(latencies.begin(), latencies.end());

    double sum = 0.0;
    for (const auto& d : latencies) {
        sum += d.count();
    }
    stats.average = sum / latencies.size();

    stats.p50 = latencies[static_cast<size_t>(latencies.size() * 0.50)].count();
    stats.p90 = latencies[static_cast<size_t>(latencies.size() * 0.90)].count();
    stats.p99 = latencies[static_cast<size_t>(latencies.size() * 0.99)].count();
    stats.p999 = latencies[static_cast<size_t>(latencies.size() * 0.999)].count();
    stats.max = latencies.back().count();

    return stats;
}

void printLatencyStats(std::ostream& os, const std::string& name, const LatencyStats& stats) {
    os << "--- " << name << " Latency (ns) ---\n"; // <-- Change unit to (ns)
    os << "Average: " << std::fixed << std::setprecision(2) << stats.average << "\n";
    os << "p50 (Median): " << stats.p50 << "\n";
    os << "p90: " << stats.p90 << "\n";
    os << "p99: " << stats.p99 << "\n";
    os << "p99.9: " << stats.p999 << "\n";
    os << "Max: " << stats.max << "\n";
    os << "-------------------------\n";
}

} // namespace Metrics