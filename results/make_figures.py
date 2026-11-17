#!/usr/bin/env python3
import csv
import statistics
from collections import defaultdict
from html import escape
from pathlib import Path


RAW = Path(__file__).resolve().parent / "raw"
FIGURES = Path(__file__).resolve().parent / "figures"
COLORS = {"SEG_MAX": "#2563eb", "SEG_MC": "#7c3aed", "FEN_B": "#dc2626",
          "ORACLE": "#16a34a", "FEN_P": "#d97706", "SEG_GCD": "#0891b2"}


def load_rows():
    path = RAW / "measurements.csv"
    if not path.exists():
        raise SystemExit("measurements.csv is missing; run results/collect_measurements.py first")
    with path.open(newline="") as stream:
        return list(csv.DictReader(stream))


def svg_start(title, width=900, height=520):
    return [f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">',
            '<rect width="100%" height="100%" fill="white"/>',
            f'<text x="{width / 2:.0f}" y="26" text-anchor="middle" font-family="sans-serif" font-size="18">{escape(title)}</text>']


def line_figure(rows, filename, title, metric, ylabel, workload_key, workload_order,
                structure_order, x_label, use_sd=False):
    values = defaultdict(list)
    for row in rows:
        if row["metric"] != metric:
            continue
        values[(row[workload_key], row["structure"])].append(float(row["observed"]))
    xs = [key for key in workload_order if any(k[0] == key for k in values)]
    structures = [name for name in structure_order if any(k[1] == name for k in values)]
    if not xs or not structures:
        return False

    summarized = {}
    maximum = 0.0
    for x in xs:
        for structure in structures:
            sample = values.get((x, structure), [])
            if not sample:
                continue
            mean = statistics.mean(sample)
            sd = statistics.stdev(sample) if len(sample) > 1 else 0.0
            summarized[(x, structure)] = (mean, sd)
            maximum = max(maximum, mean + (sd if use_sd else 0))
    maximum = max(maximum * 1.15, 1.0)

    width, height = 900, 520
    left, right, top, bottom = 90, 30, 55, 100
    plot_w, plot_h = width - left - right, height - top - bottom
    px = lambda i: left + (i * plot_w / max(1, len(xs) - 1))
    py = lambda y: top + plot_h * (1 - y / maximum)
    parts = svg_start(title, width, height)
    for tick in range(6):
        value = maximum * tick / 5
        y = py(value)
        parts.extend([f'<line x1="{left}" y1="{y:.1f}" x2="{width-right}" y2="{y:.1f}" stroke="#ddd"/>',
                      f'<text x="{left-10}" y="{y+4:.1f}" text-anchor="end" font-family="sans-serif" font-size="11">{value:.1f}</text>'])
    for i, x in enumerate(xs):
        xpos = px(i)
        parts.append(f'<text x="{xpos:.1f}" y="{height-bottom+22}" text-anchor="middle" font-family="sans-serif" font-size="11">{escape(x)}</text>')
    parts.extend([f'<line x1="{left}" y1="{height-bottom}" x2="{width-right}" y2="{height-bottom}" stroke="#222"/>',
                  f'<line x1="{left}" y1="{top}" x2="{left}" y2="{height-bottom}" stroke="#222"/>',
                  f'<text x="{left+plot_w/2:.1f}" y="{height-20}" text-anchor="middle" font-family="sans-serif" font-size="13">{escape(x_label)}</text>',
                  f'<text transform="translate(20 {top+plot_h/2:.1f}) rotate(-90)" text-anchor="middle" font-family="sans-serif" font-size="13">{escape(ylabel)}</text>'])

    for index, structure in enumerate(structures):
        color = COLORS.get(structure, "#444")
        points = []
        for i, x in enumerate(xs):
            item = summarized.get((x, structure))
            if item is None:
                continue
            mean, sd = item
            xpos = px(i)
            ypos = py(mean)
            points.append(f"{xpos:.1f},{ypos:.1f}")
            if use_sd and sd:
                parts.append(f'<line x1="{xpos:.1f}" y1="{py(max(0, mean-sd)):.1f}" x2="{xpos:.1f}" y2="{py(mean+sd):.1f}" stroke="{color}"/>')
            parts.append(f'<circle cx="{xpos:.1f}" cy="{ypos:.1f}" r="4" fill="{color}"/>')
        if points:
            parts.append(f'<polyline points="{" ".join(points)}" fill="none" stroke="{color}" stroke-width="2"/>')
        lx, ly = left + 15 + index * 155, top + 16
        parts.extend([f'<line x1="{lx}" y1="{ly}" x2="{lx+22}" y2="{ly}" stroke="{color}" stroke-width="3"/>',
                      f'<text x="{lx+28}" y="{ly+4}" font-family="sans-serif" font-size="11">{escape(structure)}</text>'])
    parts.append("</svg>")
    FIGURES.mkdir(parents=True, exist_ok=True)
    (FIGURES / filename).write_text("\n".join(parts))
    print(f"wrote results/figures/{filename}")
    return True


def ratio_figure(rows, mode, filename, title):
    grouped = defaultdict(list)
    for row in rows:
        if row["mode"] == mode and row["ratio"]:
            grouped[row["workload"]].append(float(row["ratio"]))
    if not grouped:
        return False
    cases = sorted(grouped)
    means = [statistics.mean(grouped[case]) for case in cases]
    ymax = max(1.15, max(means) * 1.15)
    width, height = 1050, 560
    left, right, top, bottom = 80, 25, 55, 190
    plot_w, plot_h = width - left - right, height - top - bottom
    py = lambda value: top + plot_h * (1 - value / ymax)
    parts = svg_start(title, width, height)
    for tick in range(6):
        value = ymax * tick / 5
        y = py(value)
        parts.extend([f'<line x1="{left}" y1="{y:.1f}" x2="{width-right}" y2="{y:.1f}" stroke="#ddd"/>',
                      f'<text x="{left-10}" y="{y+4:.1f}" text-anchor="end" font-family="sans-serif" font-size="11">{value:.2f}</text>'])
    reference_y = py(1.0)
    parts.append(f'<line x1="{left}" y1="{reference_y:.1f}" x2="{width-right}" y2="{reference_y:.1f}" stroke="#111" stroke-dasharray="5,4"/>')
    slot = plot_w / len(cases)
    for i, (case, value) in enumerate(zip(cases, means)):
        x = left + slot * (i + 0.5)
        parts.append(f'<circle cx="{x:.1f}" cy="{py(value):.1f}" r="5" fill="#2563eb"/>')
        parts.append(f'<text transform="translate({x:.1f} {height-bottom+12}) rotate(-55)" text-anchor="end" font-family="sans-serif" font-size="10">{escape(case)}</text>')
    parts.extend([f'<line x1="{left}" y1="{height-bottom}" x2="{width-right}" y2="{height-bottom}" stroke="#222"/>',
                  f'<line x1="{left}" y1="{top}" x2="{left}" y2="{height-bottom}" stroke="#222"/>',
                  f'<text x="{left+plot_w/2:.1f}" y="{height-12}" text-anchor="middle" font-family="sans-serif" font-size="12">Observed / predicted or bound (dashed line = 1)</text>',
                  "</svg>"])
    FIGURES.mkdir(parents=True, exist_ok=True)
    (FIGURES / filename).write_text("\n".join(parts))
    print(f"wrote results/figures/{filename}")
    return True


def main():
    rows = load_rows()
    generated = 0
    generated += line_figure(rows, "update_mix_timings.svg", "Mean time per operation by update fraction",
                            "time_per_operation", "Nanoseconds per operation", "workload",
                            [f"u={u:.4f}" for u in (0, 0.0005, 0.01, 0.1, 0.5, 1.0)],
                            ["SEG_MAX", "FEN_B", "ORACLE"], "Update fraction", use_sd=True)
    generated += line_figure(rows, "query_width_steps.svg", "Measured structural steps by query width",
                            "structural_steps", "Structural steps per operation", "workload",
                            [f"width={w}" for w in (16, 1024, 65536, 1000000)],
                            ["ORACLE", "SEG_MC", "SEG_MAX", "FEN_B"], "Query width (elements)")
    generated += ratio_figure(rows, "adversary", "adversary_ratios.svg", "Adversarial workload: observed / bound")
    generated += ratio_figure(rows, "gcd", "gcd_ratios.svg", "GCD workload: observed / bound")
    if not generated:
        raise SystemExit("no figures generated; collect benchmark logs for a supported mode first")


if __name__ == "__main__":
    main()
