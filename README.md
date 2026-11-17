# Project 5 — Aggregates a Fenwick Tree Cannot Support

## Project

The program maintains sensor readings and calibration factors under point assignments. For any inclusive range it reports the maximum reading, the number of times that maximum occurs, and the gcd of the calibration factors. Reading assignments may increase or decrease values.

Structure A is a generic segment tree. Structure B is the Dima–Ceterchi two-tree Fenwick range-maximum structure. An array oracle checks correctness, and an increase-only prefix-max Fenwick tree is retained as a comparison baseline.

## Build and run

From the project root, run this single command:

```bash
./run_all.sh
```

It clears prior generated outputs, builds all three executables with `g++ -std=c++17 -O2`, and then runs:

- The full test suite with 1,000,000 operations and seed 1; the differential test uses 1,000,000 elements.
- The application on the fixed 1,000,000-element input in `examples/full_scale_input.txt`.
- The `width`, `mix`, `adversary`, and `gcd` benchmark modes, each at 1,000,000 elements.

The text outputs, benchmark seed metadata, and application result file are saved under `results/raw/`. A nonzero command stops the script and returns failure.

After it finishes, create the CSV tables and SVG figures:

```bash
python3 results/collect_measurements.py
python3 results/make_figures.py
```

These Python scripts only process the saved output files. They do not build or run the app, tests, or benchmarks.

## Application input

`examples/full_scale_input.txt` is a static input file; there is no input generator. It contains one million readings, one million positive calibration factors, and seven commands. The commands demonstrate a unique maximum, a reading decrease that makes the maximum repeated, a calibration update, and subsequent range queries.

Input format:

```text
n q
n reading values
n positive calibration factors
q commands
```

Commands use 1-based indices and inclusive endpoints:

- `Q l r` — output maximum, its count, and range gcd.
- `R i x` — assign reading `i` to `x`.
- `C i x` — assign calibration factor `i` to positive value `x`.

The application writes one output line for every query. The supplied input should produce:

```text
max=10 count=1 gcd=6
max=9 count=1000000 gcd=6
max=9 count=2 gcd=2
max=9 count=1 gcd=2
```

## Owners

| Role | Member name | Main files | Derivation |
|---|---|---|---|
| Structure A owner | Vinod | `src/structA/d1_segtree.h` | D1 |
| Structure B owner | Pranjali | `src/structB/d2_fenwick.h` | D2 |
| Architect | Chahak | `app/`, `INTERFACE.md` | D3 |
| Oracle owner | Shravani | `oracle/` | D4 |
| Adversary | Kushaal | `adversary/adversary.h` | D5 |
| Analyst | Sandeep | `bench/`, `results/` | D6 |

`src/common.h` is shared infrastructure. Add member names and complete the derivations, review history, and release requirements from the project brief before submission.
