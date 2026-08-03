#!/usr/bin/env python3
import sys
import csv
import matplotlib.pyplot as plt
from collections import defaultdict

csv_path = sys.argv[1] if len(sys.argv) > 1 else "benchmark_results.csv"

data = defaultdict(lambda: {"n": [], "time": [], "timeout": []})

with open(csv_path) as f:
    reader = csv.DictReader(f)
    for row in reader:
        solver = row["solver"]
        n = int(row["n"])
        t = float(row["time_seconds"])
        is_timeout = row["result"] == "TIMEOUT"

        data[solver]["n"].append(n)
        data[solver]["time"].append(t)
        data[solver]["timeout"].append(is_timeout)

fig, ax = plt.subplots(figsize=(9, 6))

markers = {"dpll": "o", "cdcl": "s"}
colors = {"dpll": "tab:blue", "cdcl": "tab:orange"}

for solver, vals in data.items():
    ns = vals["n"]
    times = vals["time"]
    timeouts = vals["timeout"]

    ax.plot(ns, times, marker=markers.get(solver, "x"), color=colors.get(solver),
            label=solver, linewidth=1.5)

    # Timeouts separat markieren
    for n, t, is_to in zip(ns, times, timeouts):
        if is_to:
            ax.scatter([n], [t], color=colors.get(solver), marker="x", s=100, zorder=5)

ax.set_xlabel("n (Formelgroesse / Parameter)")
ax.set_ylabel("Zeit (Sekunden)")
ax.set_yscale("log")
ax.set_title("Laufzeitvergleich DPLL vs. CDCL")
ax.legend()
ax.grid(True, which="both", linestyle="--", alpha=0.5)

plt.tight_layout()
plt.savefig("benchmark_plot.png", dpi=150)
print("Plot gespeichert als benchmark_plot.png")
plt.show()