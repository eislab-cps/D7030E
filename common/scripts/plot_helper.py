#!/usr/bin/env python3
# Utility: plot_helper
# Usage: plot_helper.py <csv_file>

import matplotlib.pyplot as plt
import csv
import sys

def plot_throughput(csv_file):
    """
    Expect CSV with header and two columns: parameter,x values in first column and throughput in second.
    """
    x = []
    y = []
    with open(csv_file, newline='') as f:
        reader = csv.reader(f)
        headers = next(reader)
        for row in reader:
            x.append(float(row[0]))
            y.append(float(row[1]))
    plt.figure()
    plt.plot(x, y)
    plt.xlabel(headers[0])
    plt.ylabel(headers[1])
    plt.title(f"Throughput vs {headers[0]}")
    plt.grid(True)
    plt.show()

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <csv_file>")
        sys.exit(1)
    plot_throughput(sys.argv[1])