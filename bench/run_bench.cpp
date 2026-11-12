#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "benchmarks.h"


int main(int argc, char** argv) {
    const char* which = argc > 1 ? argv[1] : "all";
    size_t n = argc > 2 ? (size_t)std::strtoull(argv[2], nullptr, 10) : 1000000;
    const i64 V = 1000000000LL;
    if (n < 16) { std::puts("n must be at least 16"); return 2; }

    auto want = [&](const char* name) { return !std::strcmp(which, "all") || !std::strcmp(which, name); };
    int bad = 0;
    bool ran = false;


    if (want("width"))     { World W(n, V, 1); bad += benchWidth(W);     ran = true; }
    if (want("mix"))       { World W(n, V, 2); bad += benchMix(W);       ran = true; }
    if (want("adversary")) { World W(n, V, 3); bad += benchAdversary(W); ran = true; }
    if (want("gcd"))       { World W(n, V, 4); bad += benchGcd(W);       ran = true; }

    if (!ran) {
        std::puts("usage: ./bench [width|mix|adversary|gcd|all] [n]");
        return 2;
    }
    if (bad) std::printf("\n%d prediction(s) off or bound(s) violated\n", bad);
    else     std::puts("\nall predictions within tolerance, all bounds held");
    return bad ? 1 : 0;
}
