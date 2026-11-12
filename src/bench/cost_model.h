
#pragma once



#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <random>
#include <string>
#include <vector>
#include "../adversary/adversary.h"
#include "../src/common.h"

namespace d6 {

    constexpr double PHI = 1.6180339887498949;


    struct Prediction {
        std::string name;
        double query;
        double update;
        double perOp(double u) const { return (1.0 - u) * query + u * update; }
    };
    struct Check {
        std::string name;
        double predicted;
        double measured;
    };
    struct Bound {
        std::string name;
        double bound;
        double measured;
    };
    struct DecreaseModel {
        double mean;
        double worst;
    };


    inline int ceilLog2(double n) {
        size_t N = 1;
        int lg = 0;
        while ((double)N < n) { N <<= 1; ++lg; }
        return lg;
    }
    inline int floorLog2(double n) { return n < 1 ? 0 : 63 - __builtin_clzll((uint64_t)n); }
    inline double logPhi(double x) { return x <= 1 ? 0 : std::log(x) / std::log(PHI); }


    inline int segQuerySteps(size_t l, size_t r, size_t N) {
        size_t a = l + N, b = r + N + 1;
        int s = 0;
        for (; a < b; a >>= 1, b >>= 1) {
            if (a & 1) { ++a; ++s; }
            if (b & 1) { --b; ++s; }
        }
        return s;
    }



    template <class F>
    double meanOverStarts(size_t n, size_t w, F&& stepsAt) {
        w = std::max<size_t>(1, std::min(w, n));
        const size_t starts = n - w + 1;
        const size_t CAP = 4096;
        double sum = 0;
        size_t cnt = 0;
        if (starts <= CAP) {
            for (size_t l = 0; l < starts; ++l, ++cnt) sum += stepsAt(l, l + w - 1);
        } else {
            std::mt19937_64 r(12345);
            for (; cnt < CAP; ++cnt) {
                size_t l = r() % starts;
                sum += stepsAt(l, l + w - 1);
            }
        }
        return sum / (double)cnt;
    }


    inline double oracleQuery(double w) { return w; }
    inline double oracleUpdate() { return 1; }




    inline double segQueryTyp(double n, double w) {
        size_t N = 1;
        while ((double)N < n) N <<= 1;
        return meanOverStarts((size_t)n, (size_t)w,
                              [&](size_t l, size_t r) { return (double)segQuerySteps(l, r, N); });
    }

    inline double segQueryWorst(double n) { return 2.0 * ceilLog2(n); }

    inline double segUpdate(double n) { return (double)ceilLog2(n); }



    inline double fenbQueryTyp(double n, double w) {
        return meanOverStarts((size_t)n, (size_t)w, [&](size_t l, size_t r) {
            return (double)fenbQuerySteps((i64)l + 1, (i64)r + 1);
        });
    }


    inline double fenbQueryWorst(double n) { return 1.0 + 2.0 * floorLog2(n); }




    inline DecreaseModel fenbDecreaseModel(i64 n, i64 /*unused*/) {
        double sum = 0, worst = 0;
        for (i64 i = 1; i <= n; ++i) {
            double c = fenbDecreaseSteps(i, n);
            sum += c;
            worst = std::max(worst, c);
        }
        return {sum / (double)n, worst};
    }



    inline double fenpDecreaseAdversarial(double n) { return n + 0.5 * floorLog2(n) + 1.0; }





    inline double gcdQueryNaive(double n, double V) {
        return (2.0 * ceilLog2(n) + 1.0) * (logPhi(V) + 2.0);
    }



    inline double gcdQueryTight(double n, double V) {
        return 2.0 * logPhi(V) + 2.0 * (2.0 * ceilLog2(n) + 1.0);
    }

    inline double gcdUpdateTight(double n, double V) { return logPhi(V) + 2.0 * ceilLog2(n); }




    inline std::optional<double> crossoverU(const Prediction& B, const Prediction& A) {
        double dq = A.query - B.query;
        double du = B.update - A.update;
        if (dq <= 0) return std::nullopt;
        if (du <= 0) return 1.0;
        return dq / (dq + du);
    }


    inline int reportChecks(const std::vector<Check>& cs, double rel = 0.20, double absTol = 0.5) {
        int bad = 0;
        std::printf("  %-34s %12s %12s %7s\n", "case", "predicted", "measured", "ratio");
        for (const auto& c : cs) {
            double tol = std::max(absTol, rel * std::fabs(c.predicted));
            bool ok = std::fabs(c.measured - c.predicted) <= tol;
            bad += !ok;
            std::printf("  %-34s %12.2f %12.2f %7.2f  %s\n", c.name.c_str(), c.predicted, c.measured,
                        c.predicted != 0 ? c.measured / c.predicted : 0.0, ok ? "ok" : "OFF");
        }
        return bad;
    }

    inline int reportBounds(const std::vector<Bound>& bs) {
        int bad = 0;
        std::printf("  %-38s %10s %10s %8s\n", "case", "bound", "worst seen", "slack");
        for (const auto& b : bs) {
            bool ok = b.measured <= b.bound * (1 + 1e-9);
            bad += !ok;
            std::printf("  %-38s %10.2f %10.2f %8.2f  %s\n", b.name.c_str(), b.bound, b.measured,
                        b.bound - b.measured, ok ? "ok" : "VIOLATED");
        }
        return bad;
    }

}
