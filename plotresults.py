import pandas as pd
import matplotlib.pyplot as plt

# Read the benchmark results
data = pd.read_csv('benchmark_results.csv')

# Plot the results
plt.figure(figsize=(10, 6))

for lock_type in data['LockType'].unique():
    subset = data[data['LockType'] == lock_type]
    plt.plot(subset['ThreadCount'], subset['Time(ms)'], label=lock_type)

plt.xlabel('Number of Threads')
plt.ylabel('Average Time (ms)')
plt.title('Benchmark Results for Different Lock Types')
plt.legend()
plt.grid(True)
plt.savefig('benchmark_results.png')
plt.show()
