#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "application.h"

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input-file> <output-file>\n"
                  << "Input format: n q, n readings, n positive calibration factors, then q commands.\n"
                  << "Commands: Q l r | R i x | C i x (1-based indices; query endpoints inclusive).\n";
        return 2;
    }

    std::ifstream input(argv[1]);
    if (!input) {
        std::cerr << "error: cannot open input file '" << argv[1] << "'\n";
        return 2;
    }
    std::ofstream output(argv[2]);
    if (!output) {
        std::cerr << "error: cannot open output file '" << argv[2] << "'\n";
        return 2;
    }

    size_t n, q;
    if (!(input >> n >> q) || n == 0) {
        std::cerr << "error: input must begin with a positive n and operation count q\n";
        return 2;
    }

    std::vector<i64> readings(n), calibration(n);
    for (auto& value : readings) {
        if (!(input >> value)) {
            std::cerr << "error: expected " << n << " reading values\n";
            return 2;
        }
    }
    for (auto& value : calibration) {
        if (!(input >> value) || value <= 0) {
            std::cerr << "error: expected " << n << " positive calibration factors\n";
            return 2;
        }
    }

    try {
        SensorReplay app(readings, calibration);
        for (size_t op = 1; op <= q; ++op) {
            char kind;
            if (!(input >> kind)) {
                std::cerr << "error: expected command " << op << " of " << q << '\n';
                return 2;
            }
            if (kind == 'Q') {
                size_t left, right;
                if (!(input >> left >> right) || left == 0 || right == 0 || left > right || right > n) {
                    std::cerr << "error: command " << op << " needs a valid 1-based inclusive range\n";
                    return 2;
                }
                const Report result = app.ask(left - 1, right - 1);
                output << "max=" << result.max << " count=" << result.count
                       << " gcd=" << result.gcd << '\n';
            } else if (kind == 'R' || kind == 'C') {
                size_t index;
                i64 value;
                if (!(input >> index >> value) || index == 0 || index > n) {
                    std::cerr << "error: command " << op << " needs a valid 1-based index and value\n";
                    return 2;
                }
                if (kind == 'R') {
                    app.setReading(index - 1, value);
                } else {
                    if (value <= 0) {
                        std::cerr << "error: calibration factors must be positive\n";
                        return 2;
                    }
                    app.setCalibration(index - 1, value);
                }
            } else {
                std::cerr << "error: unknown command '" << kind << "' at operation " << op << '\n';
                return 2;
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }

    if (!output) {
        std::cerr << "error: failed while writing output file '" << argv[2] << "'\n";
        return 2;
    }
    return 0;
}
