#pragma once
#include <cstddef>
#include <cstdint>
#include <stdexcept>

using std::size_t;
using i64 = int64_t;

constexpr i64 NEG = INT64_MIN;

struct Stats {
    uint64_t steps = 0;
    uint64_t combines = 0;
    uint64_t euclid = 0;
    uint64_t rebuilds = 0;
    void reset() { *this = Stats(); }
};

inline i64 lowbit(i64 i) { return i & -i; }


inline void requireNonEmpty(size_t n) {
    if (n == 0) throw std::invalid_argument("array must be non-empty");
}
inline void requireIndex(size_t i, size_t n) {
    if (i >= n) throw std::out_of_range("index out of range");
}
inline void requireRange(size_t l, size_t r, size_t n) {
    if (l > r || r >= n) throw std::out_of_range("range must satisfy l <= r < n");
}

template <class S, class F>
Stats measure(S& s, F&& f) {
    s.resetStats();
    f();
    return s.stats();
}
