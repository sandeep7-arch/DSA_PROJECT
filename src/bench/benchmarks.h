#pragma once
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <random>
#include <string>
#include <vector>
#include "../adversary/adversary.h"
#include "../src/common.h"
#include "../src/structA/d1_segtree.h"
#include "../src/structB/d2_fenwick.h"
#include "cost_model.h"
#include "../oracle/oracle.h"

struct World {
    size_t n;
    i64 V;
    std::vector<i64> v, g;
    SegTree<MaxCount> sa;
    SegTree<Max> sm;
    SegTree<Gcd> sg;
    FenwickMax fb;
    Oracle ov;

    World(size_t n_, i64 V_, uint64_t seed)
        : n(n_), V(V_), v(gen(n_, 0, V_ - 1, seed)), g(gen(n_, 1, V_, seed + 1)),
          sa(v), sm(v), sg(g), fb(v), ov(v) {}

private:
    static std::vector<i64> gen(size_t n, i64 lo, i64 hi, uint64_t seed) {
        std::mt19937_64 r(seed);
        std::vector<i64> x(n);
        for (auto& e : x) e = lo + (i64)(r() % (uint64_t)(hi - lo + 1));
        return x;
    }
};

struct Run {
    Stats st;
    double ns;
};

template <class S, class F>
Run timed(S& s, F&& f) {
    s.resetStats();
    auto t0 = std::chrono::steady_clock::now();
    f();
    auto t1 = std::chrono::steady_clock::now();
    return {s.stats(), std::chrono::duration<double, std::nano>(t1 - t0).count()};
}

inline int benchWidth(World& W) {
    const size_t n = W.n;
    const int Q = 2000;
    const size_t widths[] = {16, 1024, 65536, 1000000};
    std::vector<d6::Check> checks;
    for (size_t w : widths) {
        if (w > n) continue;
        auto work = [&](auto& s, int q) {
            std::mt19937_64 r(w);
            for (int k = 0; k < q; ++k) {
                size_t l = r() % (n - w + 1);
                s.query(l, l + w - 1);
            }
        };
        int qo = w >= 65536 ? 20 : Q;
        Stats a = measure(W.sa, [&] { work(W.sa, Q); });
        Stats m = measure(W.sm, [&] { work(W.sm, Q); });
        Stats f = measure(W.fb, [&] { work(W.fb, Q); });
        Stats o = measure(W.ov, [&] { work(W.ov, qo); });
        auto add = [&](const char* name, double pred, const Stats& s, int q) {
            checks.push_back({"w=" + std::to_string(w) + " " + name, pred, (double)s.steps / q});
        };
        add("ORACLE", d6::oracleQuery((double)w), o, qo);
        add("SEG_MC", d6::segQueryTyp((double)n, (double)w), a, Q);
        add("SEG_MAX", d6::segQueryTyp((double)n, (double)w), m, Q);
        add("FEN_B", d6::fenbQueryTyp((double)n, (double)w), f, Q);
    }
    std::puts("\n[benchmark] query width: avg steps per query");
    return d6::reportChecks(checks);
}

inline int benchMix(World& W) {
    const size_t n = W.n;
    const size_t w = std::min<size_t>(1024, n);
    const int OPS = 4000;
    auto dec = d6::fenbDecreaseModel((i64)n, 20000);
    d6::Prediction pOr{"ORACLE", d6::oracleQuery((double)w), d6::oracleUpdate()};
    d6::Prediction pSeg{"SEG_MC", d6::segQueryTyp((double)n, (double)w), d6::segUpdate((double)n)};
    d6::Prediction pSegMax = pSeg;
    pSegMax.name = "SEG_MAX";
    d6::Prediction pFb{"FEN_B", d6::fenbQueryTyp((double)n, (double)w), dec.mean};

    std::vector<d6::Check> checks;
    std::puts("\n[benchmark] update mix (every update is a decrease), width 1024");
    std::puts("  u        SEG_MAX ns/op  FEN_B ns/op  ORACLE ns/op   B<SEG steps  B<SEG time");
    i64 low = -1;
    for (double u : {0.0, 0.0005, 0.01, 0.1, 0.5, 1.0}) {
        auto work = [&](auto& s) {
            std::mt19937_64 r(5);
            for (int k = 0; k < OPS; ++k) {
                if (r() % 1000000 < (uint64_t)(u * 1e6)) {
                    s.assign(r() % n, low--);
                } else {
                    size_t l = r() % (n - w + 1);
                    s.query(l, l + w - 1);
                }
            }
        };
        Run a = timed(W.sa, [&] { work(W.sa); });
        Run m = timed(W.sm, [&] { work(W.sm); });
        Run f = timed(W.fb, [&] { work(W.fb); });
        Run o = timed(W.ov, [&] { work(W.ov); });
        char buf[64];
        std::snprintf(buf, sizeof buf, "u=%.4f", u);
        std::string tag = buf;
        checks.push_back({tag + " ORACLE", pOr.perOp(u), (double)o.st.steps / OPS});
        checks.push_back({tag + " SEG_MC", pSeg.perOp(u), (double)a.st.steps / OPS});
        checks.push_back({tag + " SEG_MAX", pSegMax.perOp(u), (double)m.st.steps / OPS});
        checks.push_back({tag + " FEN_B", pFb.perOp(u), (double)f.st.steps / OPS});
        std::printf("  %-8.4f %12.1f %12.1f %13.1f   %-12s %s\n", u, m.ns / OPS, f.ns / OPS,
                    o.ns / OPS, f.st.steps < m.st.steps ? "yes" : "no",
                    f.ns < m.ns ? "yes" : "no");
    }
    auto x = d6::crossoverU(pFb, pSegMax);
    if (x) std::printf("  predicted crossover u* = %.6f\n", *x);
    else std::puts("  predicted crossover: none, B never beats SEG_MAX in steps");
    std::puts("  per-operation steps, predicted vs measured:");
    return d6::reportChecks(checks);
}

inline int benchAdversary(World& W) {
    const size_t n = W.n;
    std::vector<d6::Check> checks;
    std::vector<d6::Bound> bounds;

    auto model = d6::fenbDecreaseModel((i64)n, 20000);
    auto idx = worstDecreaseIndices(n, 16);
    auto seq = decreasingSequence(idx, 400);
    Stats b = measure(W.fb, [&] { for (auto& u : seq) W.fb.assign(u.i, u.x); });
    Stats a = measure(W.sa, [&] { for (auto& u : seq) W.sa.assign(u.i, u.x); });
    bounds.push_back({"FEN_B worst decreases steps/op", model.worst, (double)b.steps / seq.size()});
    checks.push_back({"SEG_MC same sequence steps/op", d6::segUpdate((double)n),
                      (double)a.steps / seq.size()});

    const size_t K = 100;
    auto kill = maxKiller(W.v, K);
    std::vector<i64> copy = W.v;
    PrefixMaxFenwick fp(copy);
    Stats p = measure(fp, [&] { for (auto& u : kill) fp.assign(u.i, u.x); });
    checks.push_back({"FEN_P max-killer steps/op", d6::fenpDecreaseAdversarial((double)n),
                      (double)p.steps / K});
    checks.push_back({"FEN_P max-killer rebuilds", (double)K, (double)p.rebuilds});
    Stats sk = measure(W.sa, [&] { for (auto& u : kill) W.sa.assign(u.i, u.x); });
    checks.push_back({"SEG_MC max-killer steps/op", d6::segUpdate((double)n), (double)sk.steps / K});

    auto wq = worstQueries(n, 1000);
    double mf = 0, ms = 0;
    for (auto& q : wq) {
        Stats f = measure(W.fb, [&] { W.fb.query(q.l, q.r); });
        Stats s = measure(W.sa, [&] { W.sa.query(q.l, q.r); });
        mf = std::max(mf, (double)f.steps);
        ms = std::max(ms, (double)s.steps);
    }
    bounds.push_back({"FEN_B worst query steps", d6::fenbQueryWorst((double)n), mf});
    bounds.push_back({"SEG_MC same queries steps", d6::segQueryWorst((double)n), ms});

    std::puts("\n[benchmark] adversarial inputs");
    int bad = d6::reportChecks(checks);
    bad += d6::reportBounds(bounds);
    return bad;
}

inline int benchGcd(World& W) {
    const size_t n = W.n;
    const double V = (double)W.V;
    const size_t w = std::min<size_t>(1024, n);
    std::vector<d6::Bound> bounds;
    std::mt19937_64 r(11);

    double mx = 0;
    for (int q = 0; q < 2000; ++q) {
        size_t l = r() % (n - w + 1);
        Stats s = measure(W.sg, [&] { W.sg.query(l, l + w - 1); });
        mx = std::max(mx, (double)s.euclid);
    }
    bounds.push_back({"random data query euclid (tight)", d6::gcdQueryTight((double)n, V), mx});
    bounds.push_back({"random data query euclid (naive)", d6::gcdQueryNaive((double)n, V), mx});

    SegTree<Gcd> fib(fibonacciCalibration(n));
    double mf = 0;
    for (int q = 0; q < 2000 && n > 4; ++q) {
        size_t l = 1 + 2 * (r() % ((n - 3) / 2));
        Stats s = measure(fib, [&] { fib.query(l, l + 1); });
        mf = std::max(mf, (double)s.euclid);
    }
    bounds.push_back({"fibonacci data query euclid (tight)", d6::gcdQueryTight((double)n, V), mf});

    double mu = 0;
    for (int k = 0; k < 2000; ++k) {
        size_t i = r() % n;
        i64 x = 1 + (i64)(r() % (uint64_t)W.V);
        Stats s = measure(W.sg, [&] { W.sg.assign(i, x); });
        mu = std::max(mu, (double)s.euclid);
    }
    bounds.push_back({"random update euclid (tight)", d6::gcdUpdateTight((double)n, V), mu});

    std::puts("\n[benchmark] range gcd: Euclid steps, worst observed vs derived bounds");
    return d6::reportBounds(bounds);
}
