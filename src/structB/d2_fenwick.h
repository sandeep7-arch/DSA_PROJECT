
#pragma once
#include <algorithm>
#include <cassert>
#include <vector>
#include "../common.h"

class FenwickMax {
public:
    explicit FenwickMax(const std::vector<i64>& v)
        : n(v.size()), a(n + 1, NEG), t1(n + 1, NEG), t2(n + 1, NEG) {
        requireNonEmpty(n);
        for (size_t i = 0; i < n; ++i) a[i + 1] = v[i];
        for (i64 j = 1; j <= (i64)n; ++j) recompute1(j);
        for (i64 j = (i64)n; j >= 1; --j) recompute2(j);
        st.reset();
    }

    i64 query(size_t l0, size_t r0) {
        requireRange(l0, r0, n);
        i64 L = (i64)l0 + 1, R = (i64)r0 + 1;
        int k = 63 - __builtin_clzll((uint64_t)((L - 1) ^ R));
        i64 p = (R >> k) << k;
        i64 m = a[p];
        ++st.steps;
        for (i64 c = L; c < p; c += lowbit(c)) { m = std::max(m, t2[c]); ++st.steps; }
        for (i64 c = R; c > p; c -= lowbit(c)) { m = std::max(m, t1[c]); ++st.steps; }
        return m;
    }

    void assign(size_t i0, i64 x) {
        requireIndex(i0, n);
        i64 j = (i64)i0 + 1;
        i64 old = a[j];
        if (x == old) return;
        a[j] = x;
        if (x > old) {
            for (i64 c = j; c <= (i64)n; c += lowbit(c)) { t1[c] = std::max(t1[c], x); ++st.steps; }
            for (i64 c = j; c > 0; c -= lowbit(c)) { t2[c] = std::max(t2[c], x); ++st.steps; }
        } else {
            for (i64 c = j; c <= (i64)n; c += lowbit(c)) recompute1(c);
            for (i64 c = j; c > 0; c -= lowbit(c)) recompute2(c);
        }
    }

    const Stats& stats() const { return st; }
    void resetStats() { st.reset(); }

    bool validate() const {
        for (i64 j = 1; j <= (i64)n; ++j) {
            i64 left = j - lowbit(j) + 1;
            i64 right = j + lowbit(j) - 1;
            i64 expected1 = NEG, expected2 = NEG;
            for (i64 k = left; k <= j; ++k) expected1 = std::max(expected1, a[k]);
            for (i64 k = j; k <= std::min<i64>((i64)n, right); ++k)
                expected2 = std::max(expected2, a[k]);
            if (t1[j] != expected1 || t2[j] != expected2) return false;
        }
        return true;
    }

private:
    void recompute1(i64 j) {
        i64 m = a[j];
        ++st.steps;
        for (i64 d = 1; d < lowbit(j); d <<= 1) { m = std::max(m, t1[j - d]); ++st.steps; }
        t1[j] = m;
    }
    void recompute2(i64 j) {
        i64 m = a[j];
        ++st.steps;
        for (i64 d = 1; d < lowbit(j); d <<= 1)
            if (j + d <= (i64)n) { m = std::max(m, t2[j + d]); ++st.steps; }
        t2[j] = m;
    }
    size_t n;
    std::vector<i64> a, t1, t2;
    Stats st;
};

class PrefixMaxFenwick {
public:
    explicit PrefixMaxFenwick(const std::vector<i64>& v) : n(v.size()), a(n + 1, NEG), t(n + 1, NEG) {
        requireNonEmpty(n);
        for (size_t i = 0; i < n; ++i) a[i + 1] = v[i];
        rebuild();
        st.reset();
    }

    i64 prefixQuery(size_t r0) {
        requireIndex(r0, n);
        i64 m = NEG;
        for (i64 c = (i64)r0 + 1; c > 0; c -= lowbit(c)) { m = std::max(m, t[c]); ++st.steps; }
        return m;
    }

    void assign(size_t i0, i64 x) {
        requireIndex(i0, n);
        i64 j = (i64)i0 + 1;
        i64 old = a[j];
        if (x == old) return;
        if (x > old) {
            a[j] = x;
            for (i64 c = j; c <= (i64)n; c += lowbit(c)) { t[c] = std::max(t[c], x); ++st.steps; }
            return;
        }
        bool dirty = false;
        for (i64 c = j; c <= (i64)n; c += lowbit(c)) {
            ++st.steps;
            if (t[c] == old) dirty = true;
        }
        a[j] = x;
        if (dirty) rebuild();
    }

    const Stats& stats() const { return st; }
    void resetStats() { st.reset(); }

    bool validate() const {
        for (i64 j = 1; j <= (i64)n; ++j) {
            i64 expected = NEG;
            for (i64 k = j - lowbit(j) + 1; k <= j; ++k)
                expected = std::max(expected, a[k]);
            if (t[j] != expected) return false;
        }
        return true;
    }

private:
    void rebuild() {
        t = a;
        for (i64 j = 1; j <= (i64)n; ++j) {
            i64 p = j + lowbit(j);
            if (p <= (i64)n) t[p] = std::max(t[p], t[j]);
        }
        st.steps += n;
        ++st.rebuilds;
    }
    size_t n;
    std::vector<i64> a, t;
    Stats st;
};

