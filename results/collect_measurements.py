#!/usr/bin/env python3
import csv
import re
from pathlib import Path


RAW = Path(__file__).resolve().parent / "raw"
HEADER = re.compile(r"^# ([^=]+)=(.*)$")
TABLE_ROW = re.compile(
    r"^(.+?)\s{2,}([0-9]+(?:\.[0-9]+)?)\s+"
    r"([0-9]+(?:\.[0-9]+)?)\s+([0-9]+(?:\.[0-9]+)?)\s+(ok|OFF|VIOLATED)$"
)
MIX_ROW = re.compile(
    r"^(0\.\d{4}|1\.0000)\s+([0-9.]+)\s+([0-9.]+)\s+"
    r"([0-9.]+)\s+(yes|no)\s+(yes|no)$"
)


def read_log(path):
    metadata = {}
    body = []
    for line in path.read_text().splitlines():
        match = HEADER.match(line)
        if match and not body:
            metadata[match.group(1)] = match.group(2)
        else:
            body.append(line.strip())
    return metadata, body


def measurement(meta, mode, log, workload, structure, metric, unit,
                predicted, observed, ratio="", status=""):
    return {
        "timestamp": meta.get("timestamp", ""),
        "mode": mode,
        "n": meta.get("n", "1000000"),
        "seeds": meta.get("seeds", ""),
        "workload": workload,
        "structure": structure,
        "metric": metric,
        "unit": unit,
        "predicted": predicted,
        "observed": observed,
        "ratio": ratio,
        "status": status,
        "log": log.name,
    }


def parse_benchmark(path):
    meta, lines = read_log(path)
    mode = meta.get("mode", path.stem.replace("benchmark_", "", 1))
    rows = []
    bound_table = False
    for line in lines:
        if "worst seen" in line and "slack" in line:
            bound_table = True
        elif "predicted" in line and "measured" in line and "ratio" in line:
            bound_table = False

        if mode == "mix":
            match = MIX_ROW.match(line)
            if match:
                fraction, seg, fenwick, oracle, _, _ = match.groups()
                for name, value in (("SEG_MAX", seg), ("FEN_B", fenwick), ("ORACLE", oracle)):
                    rows.append(measurement(meta, mode, path, f"u={fraction}", name,
                                            "time_per_operation", "ns/op", "", value))

        match = TABLE_ROW.match(line)
        if not match:
            continue
        label, predicted, observed, printed_ratio, status = match.groups()
        label = label.strip()
        ratio = f"{float(observed) / float(predicted):.8g}" if bound_table and float(predicted) else printed_ratio
        if mode == "width":
            width = re.search(r"w=(\d+)", label)
            workload = f"width={width.group(1)}" if width else label
            structure = label.split()[-1]
            metric, unit = "structural_steps", "steps/query"
        elif mode == "mix":
            parts = label.split()
            workload = parts[0]
            structure = parts[-1]
            metric, unit = "structural_steps", "steps/operation"
        else:
            workload = label
            structure = label.split()[0]
            if "rebuild" in label:
                metric, unit = "rebuilds", "rebuilds"
            elif mode == "gcd" or "euclid" in label:
                metric, unit = "euclidean_iterations", "iterations"
            else:
                metric, unit = "structural_steps", "steps/operation"
        rows.append(measurement(meta, mode, path, workload, structure, metric,
                                unit, predicted, observed, ratio, status))
    return meta, rows


def write_csv(path, fields, rows):
    with path.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)


def main():
    logs = sorted(RAW.glob("benchmark_*.txt"))
    if not logs:
        raise SystemExit("no benchmark logs found; run ./run_all.sh first")

    measurements_by_mode = {}
    summaries = []
    for path in logs:
        meta, rows = parse_benchmark(path)
        if not rows:
            raise SystemExit(f"could not parse benchmark output in {path}")
        measurements_by_mode.setdefault(meta.get("mode", path.stem.replace("benchmark_", "", 1)), []).extend(rows)
        summaries.append({
            "timestamp": meta.get("timestamp", ""),
            "component": meta.get("mode", path.stem),
            "n": meta.get("n", "1000000"),
            "seeds": meta.get("seeds", ""),
            "status": "passed" if "all predictions within tolerance, all bounds held" in path.read_text() else "review log",
            "log": path.name,
        })

    test_log = RAW / "tests.txt"
    if test_log.exists():
        content = test_log.read_text()
        meta, _ = read_log(test_log)
        passed = "[self-test]" in content and "passed" in content
        summaries.append({"timestamp": meta.get("timestamp", ""), "component": "tests",
                          "n": "1000000", "seeds": "data=1 event=2",
                          "status": "passed" if passed else "review log", "log": test_log.name})

    app_log = RAW / "application.log"
    app_output = RAW / "application_output.txt"
    if app_log.exists():
        meta, _ = read_log(app_log)
        query_count = len(app_output.read_text().splitlines()) if app_output.exists() else 0
        summaries.append({"timestamp": meta.get("timestamp", ""), "component": "application",
                          "n": "1000000", "seeds": "fixed input file",
                          "status": f"completed ({query_count} query results)", "log": app_output.name})

    fields = ["timestamp", "mode", "n", "seeds", "workload", "structure", "metric",
              "unit", "predicted", "observed", "ratio", "status", "log"]
    for mode, rows in measurements_by_mode.items():
        path = RAW / f"measurements_{mode}.csv"
        write_csv(path, fields, rows)
        print(f"wrote {path.relative_to(RAW.parent)} ({len(rows)} rows)")
    merged = RAW / "measurements.csv"
    if merged.exists():
        merged.unlink()
    write_csv(RAW / "run_summary.csv",
              ["timestamp", "component", "n", "seeds", "status", "log"], summaries)
    print(f"wrote results/raw/run_summary.csv ({len(summaries)} rows)")


if __name__ == "__main__":
    main()
