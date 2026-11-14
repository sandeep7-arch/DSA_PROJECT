#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <ostream>
#include <random>
#include <string>
#include <vector>
#include "application.h"

static std::vector<i64> gen(size_t n, i64 lo, i64 hi, uint64_t seed) {
    std::mt19937_64 r(seed);
    std::vector<i64> x(n);
    for (auto& e : x) e = lo + (i64)(r() % (uint64_t)(hi - lo + 1));
    return x;
}

static double ms(std::chrono::steady_clock::time_point a) {
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - a).count();
}

static int runSweep() {
    int bad = 0;
    std::puts("  n      u     width   ok   steps/op  euclid/op  seg ms  oracle ms");
    for (size_t n : {1u, 7u, 20000u})
        for (double u : {0.0, 0.1, 0.5, 1.0})
            for (size_t w : {0u, 16u, 1024u}) {
                auto v = gen(n, 0, 999999999, 1), g = gen(n, 1, 1000000000, 2);
                const size_t ops = 4000;
                auto log = makeLog(n, ops, u, w, 7);
                SensorReplay app(v, g);
                auto t0 = std::chrono::steady_clock::now();
                auto got = app.run(log);
                double tseg = ms(t0);
                t0 = std::chrono::steady_clock::now();
                auto want = replayOracle(v, g, log);
                double tor = ms(t0);
                bool ok = got == want;
                bad += !ok;
                double st = (double)(app.readingStats().steps + app.calibrationStats().steps) / ops;
                double eu = (double)app.calibrationStats().euclid / ops;
                std::printf("%6zu %5.2f %6zu   %-4s %8.1f %9.1f %8.2f %9.2f\n",
                            n, u, w, ok ? "yes" : "NO", st, eu, tseg, tor);
            }
    std::puts(bad ? "MISMATCH" : "all configurations match the oracle");
    return bad != 0;
}

static int runGeneratedDemo(size_t n, size_t operations, std::ostream& out, bool recordAll) {
    if (n == 0) {
        std::cerr << "error: n must be positive\n";
        return 2;
    }

    constexpr uint64_t seed = 20261111;
    auto readings = gen(n, 0, 999999999, seed);
    auto calibration = gen(n, 1, 1000000000, seed + 1);
    if (n >= 2) readings[0] = readings[1] = 1000000000;
    SensorReplay app(readings, calibration);
    std::mt19937_64 rng(seed + 2);
    size_t queryCount = 0;
    size_t decreaseCount = 0;
    const size_t shownQueries = 8;

    out << "Sensor replay: " << n << " values, " << operations
        << " operations, seed=" << seed << '\n';
    if (n <= 100) {
        out << "initial readings: ";
        for (i64 x : readings) out << x << ' ';
        out << "\ninitial calibration: ";
        for (i64 x : calibration) out << x << ' ';
        out << '\n';
    } else {
        out << "initial arrays are generated deterministically from the listed seed\n";
    }
    for (size_t k = 0; k < operations; ++k) {
        if (k == 0 || k == 2 || k % 5 == 2) {
            size_t l = 0;
            size_t r = n - 1;
            if (k >= 4) {
                size_t width = 1 + rng() % n;
                l = rng() % (n - width + 1);
                r = l + width - 1;
            }
            Report result = app.ask(l, r);
            if (recordAll || queryCount < shownQueries) {
                out << "op " << (k + 1) << " query [" << (l + 1) << ',' << (r + 1)
                    << "]: max=" << result.max << " count=" << result.count
                    << (result.count == 1 ? " (unique maximum)" : " (maximum is repeated)")
                    << " gcd=" << result.gcd << '\n';
            }
            ++queryCount;
        } else if (k == 1 || k % 5 == 0) {
            size_t i = decreaseCount % n;
            i64 value = -1 - (i64)decreaseCount;
            if (recordAll) out << "op " << (k + 1) << " lower reading[" << (i + 1)
                               << "] to " << value << '\n';
            app.setReading(i, value);
            ++decreaseCount;
        } else {
            size_t i = rng() % n;
            i64 value = 1 + (i64)(rng() % 1000000000);
            if (recordAll) out << "op " << (k + 1) << " set calibration[" << (i + 1)
                               << "] to " << value << '\n';
            app.setCalibration(i, value);
        }
    }
    if (!recordAll && queryCount > shownQueries)
        out << "... " << (queryCount - shownQueries) << " more query results suppressed\n";
    out << "completed: " << queryCount << " queries, " << decreaseCount
        << " reading decreases, " << (operations - queryCount - decreaseCount)
        << " calibration updates\n";
    return 0;
}

static void printUsage(const char* program) {
    std::cout << "Usage:\n"
              << "  " << program << " < input.txt\n"
              << "  " << program << " --demo [n] [operations] [output.txt]\n"
              << "  " << program << " --sweep\n\n"
              << "The default demo uses 1,000,000 values and 1,000,000 operations.\n\n"
              << "Input format (indices and ranges are 1-based and inclusive):\n"
              << "  n q\n"
              << "  n reading values\n"
              << "  n positive calibration factors\n"
              << "  q commands, one per line:\n"
              << "    Q l r     query range; print max, its count, and gcd\n"
              << "    R i x     assign reading at index i to x\n"
              << "    C i x     assign calibration factor at index i to x (x > 0)\n";
}

static int runCli() {
    size_t n, q;
    if (!(std::cin >> n >> q)) {
        printUsage("application");
        return 2;
    }
    if (n == 0) {
        std::cerr << "error: n must be positive\n";
        return 2;
    }

    std::vector<i64> readings(n), calibration(n);
    for (auto& x : readings) {
        if (!(std::cin >> x)) {
            std::cerr << "error: expected " << n << " reading values\n";
            return 2;
        }
    }
    for (auto& x : calibration) {
        if (!(std::cin >> x) || x <= 0) {
            std::cerr << "error: expected " << n << " positive calibration factors\n";
            return 2;
        }
    }

    try {
        SensorReplay app(readings, calibration);
        for (size_t op = 0; op < q; ++op) {
            char kind;
            if (!(std::cin >> kind)) {
                std::cerr << "error: expected command " << (op + 1) << " of " << q << "\n";
                return 2;
            }
            if (kind == 'Q') {
                size_t l, r;
                if (!(std::cin >> l >> r) || l == 0 || r == 0) {
                    std::cerr << "error: query requires positive 1-based endpoints\n";
                    return 2;
                }
                Report result = app.ask(l - 1, r - 1);
                std::cout << "max=" << result.max << " count=" << result.count
                          << " gcd=" << result.gcd << '\n';
            } else if (kind == 'R' || kind == 'C') {
                size_t i;
                i64 x;
                if (!(std::cin >> i >> x) || i == 0) {
                    std::cerr << "error: update requires a positive 1-based index and a value\n";
                    return 2;
                }
                if (kind == 'R') {
                    app.setReading(i - 1, x);
                    std::cout << "reading[" << i << "]=" << x << '\n';
                } else {
                    if (x <= 0) {
                        std::cerr << "error: calibration factors must be positive\n";
                        return 2;
                    }
                    app.setCalibration(i - 1, x);
                    std::cout << "calibration[" << i << "]=" << x << '\n';
                }
            } else {
                std::cerr << "error: unknown command '" << kind << "' at operation " << (op + 1) << '\n';
                return 2;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << '\n';
        return 2;
    }
    return 0;
}

int main(int argc, char** argv) {
    if (argc == 2 && std::string(argv[1]) == "--help") {
        printUsage(argv[0]);
        return 0;
    }
    if (argc == 2 && std::string(argv[1]) == "--sweep") return runSweep();
    if (argc >= 2 && argc <= 5 && std::string(argv[1]) == "--demo") {
        size_t n = argc > 2 ? (size_t)std::strtoull(argv[2], nullptr, 10) : 1000000;
        size_t operations = argc > 3 ? (size_t)std::strtoull(argv[3], nullptr, 10) : 1000000;
        if (argc > 4) {
            std::ofstream out(argv[4]);
            if (!out) {
                std::cerr << "error: cannot open output file " << argv[4] << '\n';
                return 2;
            }
            int result = runGeneratedDemo(n, operations, out, true);
            if (result == 0) std::cout << "demo log written to " << argv[4] << '\n';
            return result;
        }
        return runGeneratedDemo(n, operations, std::cout, false);
    }
    if (argc != 1) {
        printUsage(argv[0]);
        return 2;
    }
    return runCli();
}
