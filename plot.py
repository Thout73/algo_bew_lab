#!/usr/bin/env python3
import sys
import csv
import argparse
import matplotlib.pyplot as plt
from collections import defaultdict

parser = argparse.ArgumentParser(description="Plottet Benchmark-CSV (range- oder sweep-Modus)")
parser.add_argument("csv_path", nargs="?", default="benchmark_results.csv")
parser.add_argument("--mode", choices=["range", "sweep"], default="range",
                     help="range: X-Achse ist Datei-Index n. sweep: X-Achse ist der geswepte Optionswert.")
parser.add_argument("--title", default=None, help="Plot-Titel (Default je nach --mode)")
parser.add_argument("--xlabel", default=None, help="X-Achsen-Beschriftung (Default: Spaltenname aus der CSV)")
parser.add_argument("--out", default="benchmark_plot.png", help="Ausgabedatei fuer den Plot")
args = parser.parse_args()

with open(args.csv_path) as f:
    reader = csv.DictReader(f)
    fieldnames = reader.fieldnames
    # Spalten 0/1 sind immer solver,file; Spalten -2/-1 immer result,time_seconds.
    # Die X-Spalte dazwischen heisst im range-Modus "n", im sweep-Modus wie die Option
    # (z.B. "restart_after") -- wird hier generisch aus dem Header gelesen statt fest "n".
    x_col = fieldnames[2]

    data = defaultdict(lambda: {"x": [], "time": [], "timeout": []})

    for row in reader:
        solver = row["solver"]
        x_val = float(row[x_col])
        t = float(row["time_seconds"])
        is_timeout = row["result"] == "TIMEOUT"

        data[solver]["x"].append(x_val)
        data[solver]["time"].append(t)
        data[solver]["timeout"].append(is_timeout)

fig, ax = plt.subplots(figsize=(9, 6))

base_colors = ["tab:blue", "tab:orange", "tab:green", "tab:red",
               "tab:purple", "tab:brown", "tab:pink", "tab:gray"]
base_markers = ["o", "s", "^", "D", "v", "P", "X", "*"]

for i, (solver, vals) in enumerate(sorted(data.items())):
    xs = vals["x"]
    times = vals["time"]
    timeouts = vals["timeout"]

    color = base_colors[i % len(base_colors)]
    marker = base_markers[i % len(base_markers)]

    # nach x sortieren, damit Linien nicht durcheinander springen
    order = sorted(range(len(xs)), key=lambda k: xs[k])
    xs_sorted = [xs[k] for k in order]
    times_sorted = [times[k] for k in order]

    ax.plot(xs_sorted, times_sorted, marker=marker, color=color,
            label=solver, linewidth=1.5)

    for x_val, t, is_to in zip(xs, times, timeouts):
        if is_to:
            ax.scatter([x_val], [t], color=color, marker="x", s=100, zorder=5)

xlabel = args.xlabel if args.xlabel else x_col
if args.title:
    title = args.title
elif args.mode == "sweep":
    title = f"Laufzeit in Abhaengigkeit von '{x_col}'"
else:
    title = "Laufzeitvergleich"

ax.set_xlabel(xlabel)
ax.set_ylabel("Zeit (Sekunden)")
ax.set_yscale("log")
ax.set_title(title)
ax.legend()
ax.grid(True, which="both", linestyle="--", alpha=0.5)

plt.tight_layout()
plt.savefig(args.out, dpi=150)
print(f"Plot gespeichert als {args.out}")
plt.show()