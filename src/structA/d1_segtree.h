#pragma once
#include <utility>
#include <vector>
#include "common.h"

struct MaxCount {
    struct T { i64 v; i64 c; };
    using In = i64;
    static T id() { return {NEG, 0}; }
    static T leaf(In x) { return {x, 1}; }
    static T op(const T& a, const T& b, Stats&) {
        if (a.v > b.v) return a;
        if (b.v > a.v) return b;
        return {a.v, a.c + b.c};
    }
};

struct Max {
    using T = i64;
    using In = i64;
    static T id() { return NEG; }
    static T leaf(In x) { return x; }
    static T op(T a, T b, Stats&) { return a > b ? a : b; }
};

struct Gcd {
    using T = i64;
    using In = i64;
    static T id() { return 0; }
    static T leaf(In x) { return x; }
    static T op(T a, T b, Stats& s) {
        if (a == 0) return b;
        if (b == 0) return a;
        while (b) {
            a %= b;
            std::swap(a, b);
            ++s.euclid;
        }
        return a;
    }
};

template <class M>
class SegTree {
public:
    using T = typename M::T;
    using In = typename M::In;

    explicit SegTree(const std::vector<In>& a) : n(a.size()), N(1) {
        requireNonEmpty(n);
        while (N < n) N <<= 1;
        t.assign(2 * N, M::id());
        for (size_t i = 0; i < n; ++i) t[N + i] = M::leaf(a[i]);
        for (size_t i = N - 1; i >= 1; --i) t[i] = comb(t[2 * i], t[2 * i + 1]);
        st.reset();
    }

    T query(size_t l, size_t r) {
        requireRange(l, r, n);
        T L = M::id(), R = M::id();
        for (l += N, r += N + 1; l < r; l >>= 1, r >>= 1) {
            if (l & 1) { L = comb(L, t[l++]); ++st.steps; }
            if (r & 1) { R = comb(t[--r], R); ++st.steps; }
        }
        return comb(L, R);
    }

    void assign(size_t i, In x) {
        requireIndex(i, n);
        size_t p = i + N;
        t[p] = M::leaf(x);
        for (p >>= 1; p >= 1; p >>= 1) {
            t[p] = comb(t[2 * p], t[2 * p + 1]);
            ++st.steps;
        }
    }

    const Stats& stats() const { return st; }
    void resetStats() { st.reset(); }

private:
    T comb(const T& a, const T& b) {
        ++st.combines;
        return M::op(a, b, st);
    }
    size_t n;
    size_t N;
    std::vector<T> t;
    Stats st;
};
