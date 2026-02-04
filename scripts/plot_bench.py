import argparse
import csv
from collections import defaultdict
from pathlib import Path

import matplotlib.pyplot as plt


def load(paths):
    data = []
    for path in paths:
        with open(path, newline="") as f:
            reader = csv.DictReader(f)
            for row in reader:
                try:
                    data.append(
                        {
                            "n": int(row["n"]),
                            "m": int(row["m"]),
                            "directed": row.get("directed", "0") in ("1", "true", "True"),
                            "algo": row["algo"],
                            "time_ms": float(row["time_us"]) / 1000.0,
                            "source": Path(path).name,
                        }
                    )
                except (KeyError, ValueError):
                    continue
    return data


def main():
    parser = argparse.ArgumentParser(description="Plot shortest-path benchmarks (CSV from graph_cli).")
    parser.add_argument("csv", nargs="+", help="One or more bench.csv files")
    parser.add_argument("--out", help="Save plot to file instead of showing")
    args = parser.parse_args()

    data = load(args.csv)
    if not data:
        print("No data to plot")
        return

    series = defaultdict(list)  # key: (algo, directed)
    for row in data:
        key = (row["algo"], row["directed"])
        series[key].append(row)

    fig, ax = plt.subplots()
    for (algo, directed), rows in series.items():
        rows = sorted(rows, key=lambda r: r["n"])
        xs = [r["n"] for r in rows]
        ys = [r["time_ms"] for r in rows]
        label = f"{algo} ({'dir' if directed else 'undir'})"
        ax.plot(xs, ys, marker="o", label=label)

    ax.set_xlabel("Vertices (n)")
    ax.set_ylabel("Time, ms")
    ax.set_title("Shortest path benchmark")
    ax.grid(True, linestyle="--", alpha=0.4)
    ax.legend()
    plt.tight_layout()

    if args.out:
        plt.savefig(args.out, dpi=200)
        print(f"Saved to {args.out}")
    else:
        plt.show()


if __name__ == "__main__":
    main()
