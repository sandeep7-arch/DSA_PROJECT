# Common Interface and Structural Invariants

**Architect:** Chahak Dhote
**Interface version:** 1.0  



## 1. Application contract

The application reads `n q`, followed by `n` reading values, `n` positive calibration factors, and then `q` commands. File indices are 1-based; ranges are closed/inclusive.

| Command | Meaning | Output |
|---|---|---|
| `Q l r` | Query the range `[l,r]` | `max=<value> count=<multiplicity> gcd=<value>` |
| `R i x` | Assign reading `i` to signed 64-bit value `x` | No query output |
| `C i x` | Assign calibration factor `i` to positive 64-bit value `x` | No query output |

The executable takes exactly two paths: input file and output file. It writes one result line per `Q` command and reports invalid input to standard error.

## 2. Shared C++ API

Library indices are 0-based. Query ranges are closed and must satisfy `0 <= l <= r < n`; assignment indices must satisfy `0 <= i < n`. Structures reject empty arrays and throw on out-of-range operations.

`SensorReplay(readings, calibration)` requires equal-length arrays. `ask(l,r)` returns:

```cpp
struct Report {
    i64 max;
    i64 count;
    i64 gcd;
};
```

`setReading(i,x)` and `setCalibration(i,x)` perform point assignments. `run(events)` processes `Reading`, `Calibration`, and `Query` events in order and returns one `Report` for each query. The CLI additionally enforces positive calibration inputs.

## 3. Structure A — generic segment tree

`SegTree<M>` is parameterized by a monoid with `id()`, `leaf(x)`, and associative `op(a,b,stats)`. The tree pads to the next power of two with the identity. Queries combine selected nodes in left-to-right order; assignments replace one leaf and recompute its ancestors.

| Monoid | Stored value | Identity | Combine rule |
|---|---|---|---|
| `MaxCount` | `(maximum,count)` | `(INT64_MIN,0)` | Keep the larger maximum; sum counts on a tie |
| `Max` | maximum | `INT64_MIN` | Maximum of the two values |
| `Gcd` | gcd | `0` | Integer gcd, counting Euclidean iterations |

**Invariant:** each internal node stores the monoid operation applied to its two children. The leaves for positions beyond `n` contain the identity. A query returns the monoid result for exactly the requested range.

## 4. Structure B — two-tree Fenwick range maximum

`FenwickMax` stores the input in a 1-based array `a`. For `lowbit(j)`, the largest power of two dividing `j`, its auxiliary entries represent:

\[
t_1[j]=\max a[j-\operatorname{lowbit}(j)+1\ldots j],\qquad
t_2[j]=\max a[j\ldots\min(n,j+\operatorname{lowbit}(j)-1)].
\]

The query computes a pivot from the most significant set bit of `(L-1) xor R`, reads the pivot value, then covers the left and right parts with disjoint `t2` and `t1` blocks. The pivot and those blocks partition the requested range. It takes `O(log n)` time.

An increase updates the maxima along the two chains of blocks containing the changed index. A decrease recomputes those same blocks from the array and their smaller child blocks in dependency order. The invariants are therefore restored even when a former maximum is lowered. Queries take `O(log n)`; increases take `O(log n)`; decreases take `O(log^2 n)` in the worst case.

## 5. Baseline and oracle

`PrefixMaxFenwick` stores maximums over ordinary Fenwick prefix blocks. Increasing assignments update affected entries monotonically. If a decrease invalidates a stored maximum, it rebuilds the structure; its prefix query takes `O(log n)`.

`Oracle` stores plain arrays and scans query ranges directly. Its answers are the reference for differential tests: maximum, maximum with multiplicity, and gcd. `validate()` methods independently recompute stored tree/block invariants.

## 6. Measurement counters

`Stats` records structural steps, monoid combines, Euclidean iterations, and prefix-baseline rebuilds. These are operation counts, not elapsed time. Timed benchmark results are reported separately in nanoseconds per operation.
