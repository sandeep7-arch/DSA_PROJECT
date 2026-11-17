#!/usr/bin/env bash
set -euo pipefail

n=1000000
run_stamp="$(date -u +%Y%m%d_%H%M%SZ)"

rm -rf results/raw results/figures
mkdir -p build results/raw
g++ -std=c++17 -O2 oracle/run_tests.cpp -o build/tests
g++ -std=c++17 -O2 bench/run_bench.cpp -o build/bench
g++ -std=c++17 -O2 app/run_application.cpp -o build/application

{
    printf '# timestamp=%s\n# mode=all\n# n=%s\n# seed=1\n' "$run_stamp" "$n"
    ./build/tests all "$n" 1
} 2>&1 | tee results/raw/tests.txt
{
    printf '# timestamp=%s\n# input=examples/full_scale_input.txt\n' "$run_stamp"
    ./build/application examples/full_scale_input.txt results/raw/application_output.txt
} 2>&1 | tee results/raw/application.log

run_benchmark() {
    bench_mode="$1"
    case "$bench_mode" in
        width) seeds='world=1 calibration=2 query_starts=16,1024,65536,1000000 model_sample=12345' ;;
        mix) seeds='world=2 calibration=3 operations=5' ;;
        adversary) seeds='world=3 calibration=4 workloads=deterministic' ;;
        gcd) seeds='world=4 calibration=5 workload=11' ;;
    esac
    {
        printf '# timestamp=%s\n# mode=%s\n# n=%s\n# seeds=%s\n' \
            "$run_stamp" "$bench_mode" "$n" "$seeds"
        ./build/bench "$bench_mode" "$n"
    } 2>&1 | tee "results/raw/benchmark_${bench_mode}.txt"
}

run_benchmark width
run_benchmark mix
run_benchmark adversary
run_benchmark gcd
