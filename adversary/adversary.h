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

inline std::pair<int, uint64_t> maxPopcountValue(uint64_t limit, int bits) {
    int best = __builtin_popcountll(limit);
    uint64_t bestValue = limit;
    for (int bit = 0; bit < bits; ++bit) {
        if ((limit & (uint64_t{1} << bit)) == 0) continue;
        uint64_t lowerMask = bit == 0 ? 0 : ((uint64_t{1} << bit) - 1);
        uint64_t candidate = (limit & ~((uint64_t{1} << (bit + 1)) - 1)) | lowerMask;
        int count = __builtin_popcountll(candidate);
        if (count > best) {
            best = count;
            bestValue = candidate;
        }
    }
    return {best, bestValue};
}

inline std::vector<uint64_t> maxPopcountValues(uint64_t limit, int bits,
                                                int target, size_t cap) {
    std::vector<uint64_t> values;
    auto visit = [&](auto&& self, int bit, bool tight, int ones,
                     uint64_t value) -> void {
        if (values.size() >= cap || ones > target || ones + bit + 1 < target) return;
        if (bit < 0) {
            if (ones == target) values.push_back(value);
            return;
        }
        int limitBit = (int)((limit >> bit) & 1);
        int high = tight ? limitBit : 1;
        for (int take = 0; take <= high && values.size() < cap; ++take) {
            self(self, bit - 1, tight && take == limitBit, ones + take,
                 value | (uint64_t{(unsigned)take} << bit));
        }
    };
    visit(visit, bits - 1, true, 0, 0);
    return values;
}

inline std::vector<Query> worstQueries(size_t n, size_t count) {
    std::vector<Query> out;
    if (n < 2) return out;
    i64 N = (i64)n;
    int kmax = 63 - __builtin_clzll((uint64_t)N);
    int best = 0;

    auto visitBlocks = [&](auto&& visit) {
        for (int k = 0; k <= kmax; ++k) {
            uint64_t half = uint64_t{1} << k;
            uint64_t mask = half - 1;
            uint64_t stride = half << 1;
            for (uint64_t h = 0; h + half <= (uint64_t)N; h += stride) {
                uint64_t pivot = h + half;
                uint64_t leftLimit = std::min(mask, pivot - 1);
                uint64_t rightLimit = std::min(mask, (uint64_t)N - pivot);
                auto left = maxPopcountValue(leftLimit, k);
                auto right = maxPopcountValue(rightLimit, k);
                visit(k, pivot, leftLimit, rightLimit, left.first, right.first,
                      1 + left.first + right.first);
            }
        }
    };

    visitBlocks([&](int, uint64_t, uint64_t, uint64_t, int, int, int steps) {
        best = std::max(best, steps);
    });
    if (count == 0) return out;

    visitBlocks([&](int k, uint64_t pivot, uint64_t leftLimit,
                    uint64_t rightLimit, int leftBest, int rightBest, int steps) {
        if (steps != best || out.size() >= count) return;
        size_t remaining = count - out.size();
        auto leftValues = maxPopcountValues(leftLimit, k, leftBest, remaining);
        auto rightValues = maxPopcountValues(rightLimit, k, rightBest, remaining);
        for (uint64_t left : leftValues) {
            for (uint64_t right : rightValues) {
                out.push_back({(size_t)(pivot - left - 1), (size_t)(pivot + right - 1)});
                if (out.size() >= count) return;
            }
        }
    });
    return out;
}

inline std::vector<i64> fibonacciCalibration(size_t n) {
    const i64 F43 = 433494437, F44 = 701408733;
    std::vector<i64> g(n);
    for (size_t i = 0; i < n; ++i) g[i] = (i % 2) ? F44 : F43;
    return g;
}
