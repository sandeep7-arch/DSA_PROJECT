#pragma once
#include <algorithm>
#include <numeric>
#include <utility>
#include <vector>
#include "../src/common.h"

class Oracle {
public:
    struct MaxCountResult { i64 v; i64 c; };

    explicit Oracle(std::vector<i64> init) : a(std::move(init)) {}

    MaxCountResult maxCount(size_t l, size_t r) {
        requireRange(l, r, a.size());
        MaxCountResult m{NEG, 0};
        for (size_t k = l; k <= r; ++k) {
            ++st.steps;
            if (a[k] > m.v) m = {a[k], 1};
            else if (a[k] == m.v) ++m.c;
        }
        return m;
    }

    i64 query(size_t l, size_t r) {
        requireRange(l, r, a.size());
        i64 m = NEG;
        for (size_t k = l; k <= r; ++k) {
            ++st.steps;
            m = std::max(m, a[k]);
        }
        return m;
    }

    i64 gcd(size_t l, size_t r) {
        requireRange(l, r, a.size());
        i64 g = 0;
        for (size_t k = l; k <= r; ++k) {
            ++st.steps;
            g = std::gcd(g, a[k]);
        }
        return g;
    }

    void assign(size_t i, i64 x) {
        requireIndex(i, a.size());
        a[i] = x;
        ++st.steps;
    }

    const Stats& stats() const { return st; }
    void resetStats() { st.reset(); }

private:
    std::vector<i64> a;
    Stats st;
};
