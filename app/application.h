
#pragma once
#include <algorithm>
#include <random>
#include <vector>
#include "../src/common.h"
#include "../src/structA/d1_segtree.h"
#include "../src/structB/d2_fenwick.h"
#include "../oracle/oracle.h"

struct Report {
    i64 max;
    i64 count;
    i64 gcd;
    bool operator==(const Report& o) const { return max == o.max && count == o.count && gcd == o.gcd; }
};

struct Event {
    enum Kind { Reading, Calibration, Query };
    Kind kind;
    size_t i;
    size_t l;
    size_t r;
    i64 x;
};

class SensorReplay {
public:
    SensorReplay(const std::vector<i64>& readings, const std::vector<i64>& calibration)
        : maxc(readings), maxFenwick(readings), gcdt(calibration) {
        if (readings.size() != calibration.size())
            throw std::invalid_argument("readings and calibration arrays must have equal lengths");
    }

    Report ask(size_t l, size_t r) {
        auto m = maxc.query(l, r);
        const i64 fenwickMax = maxFenwick.query(l, r);
        if (fenwickMax != m.v)
            throw std::logic_error("segment tree and Fenwick maximum disagree");
        return {fenwickMax, m.c, gcdt.query(l, r)};
    }

    void setReading(size_t i, i64 x) {
        maxc.assign(i, x);
        maxFenwick.assign(i, x);
    }
    void setCalibration(size_t i, i64 x) { gcdt.assign(i, x); }

    std::vector<Report> run(const std::vector<Event>& log) {
        std::vector<Report> out;
        for (const auto& e : log) {
            if (e.kind == Event::Reading) setReading(e.i, e.x);
            else if (e.kind == Event::Calibration) setCalibration(e.i, e.x);
            else if (e.kind == Event::Query) out.push_back(ask(e.l, e.r));
            else throw std::invalid_argument("unknown event kind");
        }
        return out;
    }

    const Stats& readingStats() const { return maxc.stats(); }
    const Stats& calibrationStats() const { return gcdt.stats(); }
    void resetStats() { maxc.resetStats(); maxFenwick.resetStats(); gcdt.resetStats(); }
    bool validate() const { return maxc.validate() && maxFenwick.validate() && gcdt.validate(); }

private:
    SegTree<MaxCount> maxc;
    FenwickMax maxFenwick;
    SegTree<Gcd> gcdt;
};

inline std::vector<Report> replayOracle(const std::vector<i64>& readings,
                                        const std::vector<i64>& calibration,
                                        const std::vector<Event>& log) {
    if (readings.size() != calibration.size())
        throw std::invalid_argument("readings and calibration arrays must have equal lengths");
    Oracle ro(readings), co(calibration);
    std::vector<Report> out;
    for (const auto& e : log) {
        if (e.kind == Event::Reading) ro.assign(e.i, e.x);
        else if (e.kind == Event::Calibration) co.assign(e.i, e.x);
        else if (e.kind == Event::Query) {
            auto m = ro.maxCount(e.l, e.r);
            out.push_back({m.v, m.c, co.gcd(e.l, e.r)});
        } else throw std::invalid_argument("unknown event kind");
    }
    return out;
}

inline std::vector<Event> makeLog(size_t n, size_t ops, double updateFraction, size_t width, uint64_t seed) {
    requireNonEmpty(n);
    std::mt19937_64 rng(seed);
    std::vector<Event> log;
    log.reserve(ops);
    for (size_t k = 0; k < ops; ++k) {
        Event e{};
        if ((double)(rng() % 1000000) / 1e6 < updateFraction) {
            bool reading = rng() % 2;
            e.kind = reading ? Event::Reading : Event::Calibration;
            e.i = rng() % n;
            e.x = reading ? (i64)(rng() % 1000000000) : 1 + (i64)(rng() % 1000000000);
        } else {
            size_t w = width ? std::min(width, n) : 1 + rng() % n;
            e.kind = Event::Query;
            e.l = rng() % (n - w + 1);
            e.r = e.l + w - 1;
        }
        log.push_back(e);
    }
    return log;
}

