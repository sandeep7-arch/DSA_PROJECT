#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "tests.h"

int main(int argc, char** argv) {
    const char* mode = argc > 1 ? argv[1] : "all";
    size_t operations = argc > 2 ? (size_t)std::strtoull(argv[2], nullptr, 10) : 1000000;
    uint64_t seed = argc > 3 ? (uint64_t)std::strtoull(argv[3], nullptr, 10) : 1;
    if (argc > 4) {
        std::fprintf(stderr, "usage: %s [all|full|d1|d2] [operations] [seed]\n", argv[0]);
        return 2;
    }

    std::mt19937_64 rng(seed);
    const size_t sizes[] = {1, 2, 3, 7, 8, 200};
    if (!std::strcmp(mode, "d1")) {
        testOrderedMonoids(rng);
        for (size_t n : sizes) testD1VsOracle(n, rng);
        std::puts("[d1] segment tree operations and monoid invariants passed");
    } else if (!std::strcmp(mode, "d2")) {
        for (size_t n : sizes) testD2VsOracle(n, rng);
        std::puts("[d2] Fenwick structures and block invariants passed");
    } else if (!std::strcmp(mode, "full")) {
        testFullScale(operations, seed);
    } else if (!std::strcmp(mode, "all")) {
        selfTest(operations, seed);
    } else {
        std::fprintf(stderr, "usage: %s [all|full|d1|d2] [operations] [seed]\n", argv[0]);
        return 2;
    }
    return 0;
}
