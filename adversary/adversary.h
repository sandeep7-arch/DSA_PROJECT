#pragma once
#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>
#include "../src/common.h"

struct Update { size_t i; i64 x; };
struct Query { size_t l; size_t r; };

inline double fenbDecreaseSteps(i64 i, i64 n) {
    double c = 0;
    for (i64 j = i; j <= n; j += lowbit(j)) c += 1 + __builtin_ctzll((uint64_t)lowbit(j));
    for (i64 j = i; j > 0; j -= lowbit(j)) {
        c += 1;
        for (i64 d = 1; d < lowbit(j); d <<= 1)
            if (j + d <= n) c += 1;
    }
    return c;
}

inline std::vector<size_t> worstDecreaseIndices(size_t n, size_t count) {
    std::vector<std::pair<double, size_t>> cost;
    cost.reserve(n);
    for (size_t i = 0; i < n; ++i) cost.push_back({fenbDecreaseSteps((i64)i + 1, (i64)n), i});
    count = std::min(count, n);
    std::partial_sort(cost.begin(), cost.begin() + count, cost.end(),
                      [](auto& a, auto& b) { return a.first > b.first; });
    std::vector<size_t> out;
    for (size_t k = 0; k < count; ++k) out.push_back(cost[k].second);
    return out;
}

inline std::vector<Update> decreasingSequence(const std::vector<size_t>& indices, size_t K) {
    std::vector<Update> seq;
    for (size_t k = 0; k < K; ++k) seq.push_back({indices[k % indices.size()], -1 - (i64)k});
    return seq;
}

inline std::vector<Update> maxKiller(std::vector<i64> v, size_t K) {
    std::vector<Update> seq;
    for (size_t k = 0; k < K; ++k) {
        size_t i = std::max_element(v.begin(), v.end()) - v.begin();
        v[i] = -1 - (i64)k;
        seq.push_back({i, v[i]});
    }
    return seq;
}

inline int fenbQuerySteps(i64 L, i64 R) {
    int k = 63 - __builtin_clzll((uint64_t)((L - 1) ^ R));
    i64 p = (R >> k) << k;
    int s = 1;
    for (i64 c = L; c < p; c += lowbit(c)) ++s;
    for (i64 c = R; c > p; c -= lowbit(c)) ++s;
    return s;
}

inline std::vector<Query> worstQueries(size_t n, size_t count) {
    std::vector<Query> out;
    if (n < 2) return out;
    i64 N = (i64)n;
    int kmax = 63 - __builtin_clzll((uint64_t)N);
    i64 lo = 1LL << kmax;
    int best = 0;
    for (i64 L = 1; L <= N; ++L)
        for (i64 R = std::max(lo, L); R <= N; ++R) best = std::max(best, fenbQuerySteps(L, R));
        for (i64 L = 1; L <= N && out.size() < count; ++L)
            for (i64 R = std::max(lo, L); R <= N && out.size() < count; ++R)
                if (fenbQuerySteps(L, R) == best) out.push_back({(size_t)(L - 1), (size_t)(R - 1)});
                return out;
}


inline std::vector<i64> fibonacciCalibration(size_t n) {
    const i64 F43 = 433494437, F44 = 701408733;
    std::vector<i64> g(n);
    for (size_t i = 0; i < n; ++i) g[i] = (i % 2) ? F44 : F43;
    return g;
}
