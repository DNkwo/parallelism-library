import matplotlib.pyplot as plt
import matplotlib.style as style
style.use('seaborn-darkgrid')

# Data
parallel_workers = [2, 3, 4, 5, 6, 7, 8]
sequential_time = 38.1009
pipeline_time = 40.2357
nested_farm_times = [22.1412, 18.2088, 20.0909, 17.4074, 20.4241, 22.8568, 25.2292]
hybrid_config1_times = [40.7495, 42.2899, 44.3034, 43.9787, 46.6236, 49.0131, 49.4192]
hybrid_config2_times = [21.9197, 20.8372, 21.5218, 21.8264, 22.5989, 24.9329, 24.5682]

# Calculate speedup
nested_farm_speedup = [sequential_time / time for time in nested_farm_times]
hybrid_config1_speedup = [sequential_time / time for time in hybrid_config1_times]
hybrid_config2_speedup = [sequential_time / time for time in hybrid_config2_times]

# Create the first graph (Average Execution Time)
plt.figure(figsize=(10, 6))
plt.plot(parallel_workers, nested_farm_times, marker='o', color='#1f77b4', linewidth=2, label='Nested Farm within Pipeline')
plt.plot(parallel_workers, hybrid_config1_times, marker='o', color='#ff7f0e', linewidth=2, label='Hybrid Nested Configuration 1')
plt.plot(parallel_workers, hybrid_config2_times, marker='o', color='#2ca02c', linewidth=2, label='Hybrid Nested Configuration 2')
plt.axhline(y=sequential_time, color='#d62728', linestyle='--', linewidth=2, label='Sequential Execution')
plt.axhline(y=pipeline_time, color='#9467bd', linestyle='--', linewidth=2, label='2-Stage Pipeline')
plt.xlabel('Number of Parallel Workers', fontsize=14)
plt.ylabel('Average Execution Time (s)', fontsize=14)
plt.title('Average Execution Time vs. Number of Parallel Workers', fontsize=16)
plt.legend(fontsize=12)
plt.grid(True)
plt.tight_layout()
plt.show()

# Create the second graph (Speedup)
plt.figure(figsize=(10, 6))
plt.plot(parallel_workers, nested_farm_speedup, marker='o', color='#1f77b4', linewidth=2, label='Nested Farm within Pipeline')
plt.plot(parallel_workers, hybrid_config1_speedup, marker='o', color='#ff7f0e', linewidth=2, label='Hybrid Nested Configuration 1')
plt.plot(parallel_workers, hybrid_config2_speedup, marker='o', color='#2ca02c', linewidth=2, label='Hybrid Nested Configuration 2')
plt.axhline(y=1, color='#d62728', linestyle='--', linewidth=2, label='Sequential Execution')
plt.axhline(y=sequential_time/pipeline_time, color='#9467bd', linestyle='--', linewidth=2, label='2-Stage Pipeline')
plt.xlabel('Number of Parallel Workers', fontsize=14)
plt.ylabel('Speedup', fontsize=14)
plt.title('Speedup vs. Number of Parallel Workers', fontsize=16)
plt.legend(fontsize=12)
plt.grid(True)
plt.tight_layout()
plt.show()