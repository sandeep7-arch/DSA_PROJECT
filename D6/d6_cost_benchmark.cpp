// D6: A small reproducible comparison of range-maximum query approaches.
// Usage: ./d6_benchmark [array-size] [queries-per-run] [seed]
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

class MaxSegmentTree {
private:
    int n;
    vector<int> tree;

    void build(int node, int left, int right, const vector<int>& a) {
        if (left == right) { tree[node] = a[left]; return; }
        int middle = (left + right) / 2;
        build(node * 2, left, middle, a);
        build(node * 2 + 1, middle + 1, right, a);
        tree[node] = tree[node * 2] > tree[node * 2 + 1] ? tree[node * 2] : tree[node * 2 + 1];
    }

    int query(int node, int left, int right, int ql, int qr) const {
        if (ql <= left && right <= qr) return tree[node];
        int middle = (left + right) / 2;
        if (qr <= middle) return query(node * 2, left, middle, ql, qr);
        if (ql > middle) return query(node * 2 + 1, middle + 1, right, ql, qr);
        int a = query(node * 2, left, middle, ql, qr);
        int b = query(node * 2 + 1, middle + 1, right, ql, qr);
        return a > b ? a : b;
    }

public:
    MaxSegmentTree(const vector<int>& a) : n((int)a.size()), tree(4 * a.size()) {
        build(1, 0, n - 1, a);
    }
    int rangeMaximum(int left, int right) const { return query(1, 0, n - 1, left, right); }
};

struct Range { int left; int right; };

int main(int argc, char* argv[]) {
    int n = argc > 1 ? atoi(argv[1]) : 20000;
    int queryCount = argc > 2 ? atoi(argv[2]) : 20000;
    unsigned int seed = argc > 3 ? (unsigned int)atoi(argv[3]) : 253;
    if (n <= 0 || queryCount <= 0) {
        cerr << "Array size and query count must be positive.\n";
        return 1;
    }
    srand(seed);

    vector<int> values(n);
    for (int i = 0; i < n; i++) values[i] = rand() % 1000000;
    vector<Range> ranges(queryCount);
    for (int i = 0; i < queryCount; i++) {
        int left = rand() % n;
        int right = rand() % n;
        if (left > right) { int temp = left; left = right; right = temp; }
        ranges[i].left = left;
        ranges[i].right = right;
    }

    MaxSegmentTree segmentTree(values);
    long long segmentChecksum = 0;
    clock_t start = clock();
    for (int i = 0; i < queryCount; i++)
        segmentChecksum += segmentTree.rangeMaximum(ranges[i].left, ranges[i].right);
    double segmentSeconds = (double)(clock() - start) / CLOCKS_PER_SEC;

    start = clock();
    long long arrayChecksum = 0;
    for (int i = 0; i < queryCount; i++) {
        int answer = values[ranges[i].left];
        for (int j = ranges[i].left + 1; j <= ranges[i].right; j++)
            if (values[j] > answer) answer = values[j];
        arrayChecksum += answer;
    }
    double arraySeconds = (double)(clock() - start) / CLOCKS_PER_SEC;

    if (segmentChecksum != arrayChecksum) {
        cerr << "The two methods returned different answers.\n";
        return 2;
    }

    cout << "size,queries,seed,segment_tree_seconds,array_scan_seconds,checksum\n";
    cout << n << "," << queryCount << "," << seed << ","
         << segmentSeconds << "," << arraySeconds << "," << segmentChecksum << "\n";
    return 0;
}
