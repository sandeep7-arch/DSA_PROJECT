#pragma once
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <random>
#include <vector>
#include "../app/application.h"
#include "../adversary/adversary.h"
#include "../src/structA/d1_segtree.h"
#include "../src/structB/d2_fenwick.h"
#include "oracle.h"

#define CHECK(c) do { if (!(c)) { std::fprintf(stderr, "CHECK failed: %s (%s:%d)\n", #c, __FILE__, __LINE__); std::abort(); } } while (0)

template <class F>
bool throws(F&& f) {
    try { f(); } catch (const std::exception&) { return true; }
    return false;
}

struct First {
    struct T { bool has; i64 v; };
    using In = i64;
    static T id() { return {false, 0}; }
    static T leaf(In x) { return {true, x}; }
    static T op(const T& a, const T& b, Stats&) { return a.has ? a : b; }
    static bool equal(const T& a, const T& b) { return a.has == b.has && (!a.has || a.v == b.v); }
};

struct Last {
    struct T { bool has; i64 v; };
    using In = i64;
    static T id() { return {false, 0}; }
    static T leaf(In x) { return {true, x}; }
    static T op(const T& a, const T& b, Stats&) { return b.has ? b : a; }
    static bool equal(const T& a, const T& b) { return a.has == b.has && (!a.has || a.v == b.v); }
};

inline void testOrderedMonoids(std::mt19937_64& rng) {
    for (size_t n : {1u, 2u, 3u, 5u, 6u, 7u, 9u, 100u, 1000u}) {
        std::vector<i64> v(n);
        for (auto& x : v) x = (i64)(rng() % 1000);
        SegTree<First> sf(v);
        SegTree<Last> sl(v);
        CHECK(sf.validate() && sl.validate());
        for (int it = 0; it < 2000; ++it) {
            if (rng() % 2) {
                size_t i = rng() % n;
                i64 x = (i64)(rng() % 1000);
                v[i] = x;
                sf.assign(i, x);
                sl.assign(i, x);
            } else {
                size_t l = rng() % n, r = rng() % n;
                if (l > r) std::swap(l, r);
                CHECK(sf.query(l, r).v == v[l]);
                CHECK(sl.query(l, r).v == v[r]);
            }
            if (it % 127 == 0) CHECK(sf.validate() && sl.validate());
        }
    }
}

inline i64 randomReading(std::mt19937_64& rng) {
    switch (rng() % 5) {
        case 0: return -1000000000LL - (i64)(rng() % 1000000);
        case 1: return 1000000000LL + (i64)(rng() % 1000000);
        default: return (i64)(rng() % 2001) - 1000;
    }
}

inline void testAgainstOracle(size_t n, std::mt19937_64& rng, size_t operations = 20000) {
    std::vector<i64> v(n), g(n);
    for (auto& x : v) x = randomReading(rng);
    for (auto& x : g) x = 1 + (i64)(rng() % 1000000000);

    SegTree<MaxCount> sa(v);
    SegTree<Max> sm(v);
    SegTree<Gcd> sg(g);
    FenwickMax fb(v);
    PrefixMaxFenwick fp(v);
    Oracle ov(v), og(g);
    CHECK(sa.validate() && sm.validate() && sg.validate());
    CHECK(fb.validate() && fp.validate());

    for (size_t it = 0; it < operations; ++it) {
        if (rng() % 2) {
            size_t i = rng() % n;
            i64 x = randomReading(rng);
            i64 y = 1 + (i64)(rng() % 1000000000);
            sa.assign(i, x);
            sm.assign(i, x);
            fb.assign(i, x);
            fp.assign(i, x);
            ov.assign(i, x);
            sg.assign(i, y);
            og.assign(i, y);
        } else {
            size_t l = rng() % n, r = rng() % n;
            if (l > r) std::swap(l, r);
            auto want = ov.maxCount(l, r);
            auto got = sa.query(l, r);
            CHECK(got.v == want.v && got.c == want.c);
            CHECK(sm.query(l, r) == want.v);
            CHECK(fb.query(l, r) == want.v);
            CHECK(fp.prefixQuery(r) == ov.query(0, r));
            CHECK(sg.query(l, r) == og.gcd(l, r));
        }
        if (it % 127 == 0) {
            CHECK(sa.validate() && sm.validate() && sg.validate());
            CHECK(fb.validate() && fp.validate());
        }
    }
    CHECK(sa.validate() && sm.validate() && sg.validate());
    CHECK(fb.validate() && fp.validate());
}

inline void testGuards() {
    std::vector<i64> v = {3, 1, 4, 1, 5};
    std::vector<i64> none;
    CHECK(throws([&] { SegTree<MaxCount> s(none); }));
    CHECK(throws([&] { SegTree<Gcd> s(none); }));
    CHECK(throws([&] { FenwickMax s(none); }));
    CHECK(throws([&] { PrefixMaxFenwick s(none); }));
    CHECK(throws([&] { SensorReplay s(v, none); }));

    SegTree<MaxCount> sa(v);
    SegTree<Gcd> sg(v);
    FenwickMax fb(v);
    PrefixMaxFenwick fp(v);
    Oracle ov(v);
    CHECK(throws([&] { sa.query(3, 2); }));
    CHECK(throws([&] { sa.query(0, 5); }));
    CHECK(throws([&] { sa.assign(5, 1); }));
    CHECK(throws([&] { sg.query(4, 9); }));
    CHECK(throws([&] { sg.assign(9, 1); }));
    CHECK(throws([&] { fb.query(3, 2); }));
    CHECK(throws([&] { fb.query(0, 5); }));
    CHECK(throws([&] { fb.assign(5, 1); }));
    CHECK(throws([&] { fp.prefixQuery(5); }));
    CHECK(throws([&] { fp.assign(5, 1); }));
    CHECK(throws([&] { ov.maxCount(2, 1); }));
    CHECK(throws([&] { ov.gcd(0, 5); }));
    CHECK(throws([&] { ov.assign(5, 1); }));
    CHECK(!throws([&] { sa.query(0, 4); }));
    CHECK(!throws([&] { fb.query(4, 4); }));
    CHECK(throws([&] { SensorReplay s(v, v); s.run({{static_cast<Event::Kind>(99), 0, 0, 0, 0}}); }));
}

inline void testAdversarial() {
    const size_t n = 4096;
    std::vector<i64> values(n);
    for (size_t i = 0; i < n; ++i) values[i] = (i64)(i * 17 + 3);
    FenwickMax fenwick(values);
    SegTree<Max> seg(values);
    Oracle oracle(values);

    auto indices = worstDecreaseIndices(n, 32);
    auto decreases = decreasingSequence(indices, 256);
    for (const auto& update : decreases) {
        fenwick.assign(update.i, update.x);
        seg.assign(update.i, update.x);
        oracle.assign(update.i, update.x);
    }
    CHECK(fenwick.validate() && seg.validate());

    auto queries = worstQueries(n, 256);
    CHECK(!queries.empty());
    for (const auto& query : queries) {
        i64 expected = oracle.query(query.l, query.r);
        CHECK(fenwick.query(query.l, query.r) == expected);
        CHECK(seg.query(query.l, query.r) == expected);
    }

    std::vector<i64> prefixValues(n);
    for (size_t i = 0; i < n; ++i) prefixValues[i] = (i64)i;
    PrefixMaxFenwick prefix(prefixValues);
    Oracle prefixOracle(prefixValues);
    auto killers = maxKiller(prefixValues, 100);
    for (const auto& update : killers) {
        prefix.assign(update.i, update.x);
        prefixOracle.assign(update.i, update.x);
        CHECK(prefix.validate());
        CHECK(prefix.prefixQuery(n - 1) == prefixOracle.query(0, n - 1));
    }
    CHECK(prefix.stats().rebuilds == killers.size());
}

inline bool replayMatches(const std::vector<i64>& readings,
                          const std::vector<i64>& calibration,
                          const std::vector<Event>& log) {
    try {
        SensorReplay app(readings, calibration);
        return app.run(log) == replayOracle(readings, calibration, log);
    } catch (const std::exception&) {
        return false;
    }
}

inline std::vector<Event> shrinkFailingLog(const std::vector<i64>& readings,
                                           const std::vector<i64>& calibration,
                                           std::vector<Event> log) {
    size_t granularity = 2;
    while (log.size() >= 2) {
        size_t chunk = (log.size() + granularity - 1) / granularity;
        bool reduced = false;
        for (size_t start = 0; start < log.size(); start += chunk) {
            size_t end = std::min(log.size(), start + chunk);
            std::vector<Event> candidate;
            candidate.reserve(log.size() - (end - start));
            candidate.insert(candidate.end(), log.begin(), log.begin() + start);
            candidate.insert(candidate.end(), log.begin() + end, log.end());
            if (!candidate.empty() && !replayMatches(readings, calibration, candidate)) {
                log.swap(candidate);
                granularity = std::max<size_t>(2, granularity - 1);
                reduced = true;
                break;
            }
        }
        if (reduced) continue;
        if (granularity >= log.size()) break;
        granularity = std::min(log.size(), granularity * 2);
    }
    return log;
}

inline bool fenwickReplayMatches(const std::vector<i64>& readings,
                                 const std::vector<Event>& log) {
    try {
        FenwickMax fenwick(readings);
        Oracle oracle(readings);
        for (const Event& e : log) {
            if (e.kind == Event::Reading) {
                fenwick.assign(e.i, e.x);
                oracle.assign(e.i, e.x);
            } else if (e.kind == Event::Query) {
                if (fenwick.query(e.l, e.r) != oracle.query(e.l, e.r)) return false;
            } else if (e.kind != Event::Calibration) {
                return false;
            }
        }
        return fenwick.validate();
    } catch (const std::exception&) {
        return false;
    }
}

inline std::vector<Event> shrinkFenwickFailingLog(const std::vector<i64>& readings,
                                                  std::vector<Event> log) {
    size_t granularity = 2;
    while (log.size() >= 2) {
        size_t chunk = (log.size() + granularity - 1) / granularity;
        bool reduced = false;
        for (size_t start = 0; start < log.size(); start += chunk) {
            size_t end = std::min(log.size(), start + chunk);
            std::vector<Event> candidate;
            candidate.reserve(log.size() - (end - start));
            candidate.insert(candidate.end(), log.begin(), log.begin() + start);
            candidate.insert(candidate.end(), log.begin() + end, log.end());
            if (!candidate.empty() && !fenwickReplayMatches(readings, candidate)) {
                log.swap(candidate);
                granularity = std::max<size_t>(2, granularity - 1);
                reduced = true;
                break;
            }
        }
        if (reduced) continue;
        if (granularity >= log.size()) break;
        granularity = std::min(log.size(), granularity * 2);
    }
    return log;
}

inline void printEvent(const Event& e, size_t index) {
    if (index == 0) std::fprintf(stderr, "minimal failing event sequence (internal indexes are zero-based):\n");
    std::fprintf(stderr, "  %zu: ", index + 1);
    if (e.kind == Event::Reading) std::fprintf(stderr, "R %zu %lld\n", e.i, (long long)e.x);
    else if (e.kind == Event::Calibration) std::fprintf(stderr, "C %zu %lld\n", e.i, (long long)e.x);
    else std::fprintf(stderr, "Q %zu %zu\n", e.l, e.r);
}

inline void testFullScale(size_t operations, uint64_t seed) {
    const size_t n = 1000000;
    std::mt19937_64 rng(seed);
    std::vector<i64> readings(n), calibration(n);
    for (auto& x : readings) x = (i64)(rng() % 1000000000);
    for (auto& x : calibration) x = 1 + (i64)(rng() % 1000000000);
    const auto initialReadings = std::move(readings);
    const auto initialCalibration = std::move(calibration);
    std::vector<Event> log = makeLog(n, operations, 0.4, 64, seed + 1);
    size_t forced = std::min<size_t>(1024, operations);
    for (size_t k = 0; k < forced; ++k)
        log[k] = {Event::Reading, k, 0, 0, -1 - (i64)k};
    if (operations > forced) {
        log[forced] = {Event::Query, 0, 0, n - 1, 0};
        if (operations > forced + 1) log[forced + 1] = {Event::Query, 0, n - 1, n - 1, 0};
    }

    SensorReplay app(initialReadings, initialCalibration);
    auto got = app.run(log);
    auto want = replayOracle(initialReadings, initialCalibration, log);
    if (got != want) {
        std::fprintf(stderr, "full-scale differential test failed; shrinking %zu operations\n", log.size());
        auto minimal = shrinkFailingLog(initialReadings, initialCalibration, std::move(log));
        std::fprintf(stderr, "reduced failing sequence (%zu operations):\n", minimal.size());
        for (size_t i = 0; i < minimal.size(); ++i) printEvent(minimal[i], i);
        std::abort();
    }
    CHECK(app.validate());

    if (!fenwickReplayMatches(initialReadings, log)) {
        std::fprintf(stderr, "full-scale Fenwick differential/invariant check failed; shrinking %zu operations\n", log.size());
        auto minimal = shrinkFenwickFailingLog(initialReadings, std::move(log));
        std::fprintf(stderr, "reduced failing sequence (%zu operations):\n", minimal.size());
        for (size_t i = 0; i < minimal.size(); ++i) printEvent(minimal[i], i);
        std::abort();
    }
    std::printf("[full-scale] n=%zu operations=%zu data_seed=%llu event_seed=%llu: segment tree application and Fenwick range max match oracle\n",
                n, operations, (unsigned long long)seed, (unsigned long long)(seed + 1));
}

inline void selfTest(size_t fullScaleOperations = 1000000, uint64_t seed = 1) {
    std::mt19937_64 rng(seed);
    testOrderedMonoids(rng);
    for (size_t n : {1u, 2u, 3u, 7u, 8u, 200u}) testAgainstOracle(n, rng);
    testGuards();
    testAdversarial();

    const size_t n = 200;
    std::vector<i64> v0(n), g0(n);
    for (auto& x : v0) x = randomReading(rng);
    for (auto& x : g0) x = 1 + (i64)(rng() % 1000000000);
    auto log = makeLog(n, 20000, 0.3, 0, seed + 2);
    CHECK(replayMatches(v0, g0, log));

    testFullScale(fullScaleOperations, seed);
    std::puts("[self-test] monoids, invariants, guards, differential replay, and full-scale checks passed");
}

inline void testD1VsOracle(size_t n, std::mt19937_64& rng) {
    testAgainstOracle(n, rng);
}

inline void testD2VsOracle(size_t n, std::mt19937_64& rng) {
    std::vector<i64> v(n);
    for (auto& x : v) x = randomReading(rng);
    FenwickMax fb(v);
    PrefixMaxFenwick fp(v);
    Oracle ov(v);
    CHECK(fb.validate() && fp.validate());
    for (int it = 0; it < 20000; ++it) {
        if (rng() % 2) {
            size_t i = rng() % n;
            i64 x = randomReading(rng);
            fb.assign(i, x);
            fp.assign(i, x);
            ov.assign(i, x);
        } else {
            size_t l = rng() % n, r = rng() % n;
            if (l > r) std::swap(l, r);
            CHECK(fb.query(l, r) == ov.query(l, r));
            CHECK(fp.prefixQuery(r) == ov.query(0, r));
        }
        if (it % 127 == 0) CHECK(fb.validate() && fp.validate());
    }
    CHECK(fb.validate() && fp.validate());
}
