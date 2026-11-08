# D6 Report: Predicting the Cost Before Timing

**Author:** Replace with the responsible team member's name  
**Project:** CS253 / MA253 Lab Project 5  
**Program:** `src/d6_cost_benchmark.cpp`

## 1. Why write a prediction first?

A timing number by itself does not explain an algorithm. If one program takes longer, we want a reason we can predict before looking at the stopwatch. The D6 cost model does that: it counts how the work grows as the array and query range get larger. The benchmark then provides measured evidence for one machine and one workload.

This report compares two ways to answer range-maximum queries after one fixed array has been built: a segment tree and a plain array scan. The plain scan is intentionally simple and acts as a useful oracle on small inputs. The benchmark does not implement the research-level Dima–Ceterchi Fenwick structure, so it cannot answer whether that structure wins.

## 2. The two approaches in everyday terms

### Segment tree

The segment tree stores the maximum of many short intervals, then larger intervals assembled from them. For a requested range, it selects a small collection of stored intervals that exactly cover the requested indices. It does not inspect every sensor in the range.

For an array with `n` positions, the tree has about `2n` to `4n` stored slots. Its construction visits each slot a constant number of times, so building takes `O(n)`. A query uses at most a small number of intervals at each level of a tree with about `log2(n)` levels, so a query takes `O(log n)`.

### Plain array scan

The scan starts at the first requested value and checks each value through the end of the range. If the range has width `w`, it performs about `w` comparisons, so it takes `O(w)`. For a tiny range, this can be very fast because the code is simple and memory is accessed in order. For a range covering most of the array, it examines almost every value.

## 3. Predicted cost for a workload

Let `n` be the number of values, `q` the number of range queries, and `w_i` the width of query `i`. For the benchmark's query-only workload:

| Method | One-time build | One query | All q queries | Extra space |
|---|---:|---:|---:|---:|
| Segment tree | `O(n)` | `O(log n)` | `O(n + q log n)` | `O(n)` |
| Plain array scan | `O(n)` to read values | `O(w_i)` | `O(n + sum(w_i))` | `O(1)` beyond the array |

If a point assignment is added, the segment tree follows one path from the changed leaf to the root, updating `O(log n)` stored answers. The direct array simply changes one slot in `O(1)`, but then every later query still scans its requested range.

The practical prediction is not “the tree always wins.” On narrow ranges, a scan may be competitive or faster. As ranges become wider or more numerous, the scan repeats work on values already visited by earlier queries, while the tree reuses its stored interval summaries. The crossover depends on range width, query count, compiler, and machine.

## 4. What the benchmark records

The program makes a seeded array and a seeded list of inclusive ranges, then runs the same ranges through both methods. It prints one CSV header and one row: array size, query count, seed, segment-tree CPU seconds, scan CPU seconds, and a checksum to keep the answers observable to the compiler.

Example:

```sh
g++ -std=c++11 -O2 -Wall -Wextra -pedantic src/d6_cost_benchmark.cpp -o d6_benchmark
./d6_benchmark 20000 20000 253
```

The arguments are array size, query count, and random seed. Use `-O2` consistently for measurement. Repeat the command at least five times for each chosen configuration and report the mean and spread. Change one factor at a time when possible: for example, keep `n` and `q` fixed while changing range widths, then keep the range distribution fixed while increasing `n`.

## 5. Reading the output responsibly

The CSV values are measurements from the current machine, not universal constants. CPU time can vary with machine load, compiler, optimization flags, and cache behavior. A single run is a demonstration, not a stable conclusion. Save the raw rows, keep the seed, and record the compiler and machine description in the final team report.

The benchmark currently uses random endpoints. Random ranges can have a mix of short and long widths, so the average width matters. For a stronger comparison, add controlled cases: width 1, a fixed medium width, and nearly the whole array. Keep those cases deterministic and label them clearly in the output.

## 6. GCD and the other project variants

The main application also uses a segment tree for range gcd. Gcd combines two values with Euclid's remainder method. A tree query visits `O(log n)` stored intervals; each gcd combination has a value-dependent cost, bounded by `O(log V)` for values up to magnitude `V`. A simple conservative query model is `O(log n log V)`. D3 in the derivations explains the tighter aggregate argument requested by the project.

The prefix-maximum Fenwick baseline handles prefix queries and increases, but it does not directly answer arbitrary range maximum by subtracting two prefixes. It also cannot safely apply decreases without rebuilding. The six-project comparison should eventually include the required Dima–Ceterchi two-tree variant, the segment tree, the prefix-only baseline, and the plain-array oracle. This benchmark currently covers only the segment tree and array scan.

## 7. Presentation summary

“Before measuring, we predict that a segment-tree query grows with the number of levels, about `log2(n)`. A scan grows with the requested range width. A scan can be a good choice for short ranges, but it repeats comparisons for overlapping queries. We use a fixed seed to make the ranges repeatable, record CSV output, and run several trials because timing depends on the machine. This result compares the segment tree to an oracle; it does not measure the advanced two-tree Fenwick method.”
