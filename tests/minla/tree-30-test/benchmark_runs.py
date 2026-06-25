#!/usr/bin/env python3
import subprocess
import os
import sys
import argparse

def main():
    parser = argparse.ArgumentParser(description="Benchmark MinLATest execution times over multiple runs.")
    parser.add_argument(
        "-n",
        type=int,
        default=100,
        help="Number of runs to execute the binary (default: 100)"
    )
    args = parser.parse_args()

    binary_path = "./build/bin/MinLATest"
    if not os.path.exists(binary_path):
        print(f"Error: Binary '{binary_path}' not found. Please build it first in the current directory.", file=sys.stderr)
        sys.exit(1)

    runs = args.n
    if runs <= 0:
        print("Error: The number of runs (-n) must be a positive integer.", file=sys.stderr)
        sys.exit(1)

    times = []

    print(f"Running {binary_path} {runs} times...")

    for i in range(runs):
        try:
            # Execute the binary
            result = subprocess.run(
                [binary_path],
                capture_output=True,
                text=True,
                check=True
            )
            # Parse the output
            lines = [line.strip() for line in result.stdout.strip().split("\n") if line.strip()]
            if len(lines) < 2:
                print(f"Error on run {i+1}: unexpected output format: {result.stdout.strip()}", file=sys.stderr)
                continue
            
            # The second line or the last line is the time value in nanoseconds
            time_val = int(lines[-1])
            times.append(time_val)
        except (subprocess.CalledProcessError, ValueError, IndexError) as e:
            print(f"Error on run {i+1}: {e}", file=sys.stderr)

    if not times:
        print("Error: No successful runs captured.", file=sys.stderr)
        sys.exit(1)

    average_time = sum(times) / len(times)
    print(f"\nExecution statistics over {len(times)} runs:")
    print(f"Average execution time: {average_time:.2f} ns")
    print(f"Min execution time: {min(times)} ns")
    print(f"Max execution time: {max(times)} ns")

if __name__ == "__main__":
    main()
