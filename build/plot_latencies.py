import matplotlib.pyplot as plt

# Load latencies from CSV
with open('/Users/jemilpatel/Documents/Projects/Order Book Simulation/build/add_latencies.csv', 'r') as f:
    add_latencies = [float(line.strip()) for line in f if line.strip()]

with open('/Users/jemilpatel/Documents/Projects/Order Book Simulation/build/match_latencies.csv', 'r') as f:
    match_latencies = [float(line.strip()) for line in f if line.strip()]

# Plot series
plt.figure(figsize=(10, 6))
plt.plot(add_latencies, label='Add Latency (us)', color='blue')
plt.plot(match_latencies, label='Match Latency (us)', color='green')
plt.xlabel('Operation Index')
plt.ylabel('Latency (microseconds)')
plt.title('Order Book Operation Latencies Over Time')
plt.legend()
plt.grid(True)
plt.savefig('latencies.png')
plt.show()  # Optional: display interactively