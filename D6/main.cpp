#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>

using namespace std;

struct MaxAnswer {
    int value;
    int count;
};

MaxAnswer combineMax(MaxAnswer a, MaxAnswer b) {
    if (a.value > b.value) return a;
    if (b.value > a.value) return b;
    MaxAnswer answer;
    answer.value = a.value;
    answer.count = a.count + b.count;
    return answer;
}

int gcdValue(int a, int b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b != 0) {
        int remainder = a % b;
        a = b;
        b = remainder;
    }
    return a;
}

class SegmentTree {
private:
    int n;
    vector<MaxAnswer> maximumTree;
    vector<int> gcdTree;

    void build(int node, int left, int right, const vector<int>& values) {
        if (left == right) {
            maximumTree[node].value = values[left];
            maximumTree[node].count = 1;
            gcdTree[node] = abs(values[left]);
            return;
        }
        int middle = (left + right) / 2;
        build(node * 2, left, middle, values);
        build(node * 2 + 1, middle + 1, right, values);
        maximumTree[node] = combineMax(maximumTree[node * 2], maximumTree[node * 2 + 1]);
        gcdTree[node] = gcdValue(gcdTree[node * 2], gcdTree[node * 2 + 1]);
    }

    void assignValue(int node, int left, int right, int index, int value) {
        if (left == right) {
            maximumTree[node].value = value;
            maximumTree[node].count = 1;
            gcdTree[node] = abs(value);
            return;
        }
        int middle = (left + right) / 2;
        if (index <= middle) assignValue(node * 2, left, middle, index, value);
        else assignValue(node * 2 + 1, middle + 1, right, index, value);
        maximumTree[node] = combineMax(maximumTree[node * 2], maximumTree[node * 2 + 1]);
        gcdTree[node] = gcdValue(gcdTree[node * 2], gcdTree[node * 2 + 1]);
    }

    MaxAnswer queryMaximum(int node, int left, int right, int ql, int qr) const {
        if (ql <= left && right <= qr) return maximumTree[node];
        int middle = (left + right) / 2;
        if (qr <= middle) return queryMaximum(node * 2, left, middle, ql, qr);
        if (ql > middle) return queryMaximum(node * 2 + 1, middle + 1, right, ql, qr);
        return combineMax(queryMaximum(node * 2, left, middle, ql, qr),
                          queryMaximum(node * 2 + 1, middle + 1, right, ql, qr));
    }

    int queryGcd(int node, int left, int right, int ql, int qr) const {
        if (ql <= left && right <= qr) return gcdTree[node];
        int middle = (left + right) / 2;
        if (qr <= middle) return queryGcd(node * 2, left, middle, ql, qr);
        if (ql > middle) return queryGcd(node * 2 + 1, middle + 1, right, ql, qr);
        return gcdValue(queryGcd(node * 2, left, middle, ql, qr),
                        queryGcd(node * 2 + 1, middle + 1, right, ql, qr));
    }

public:
    SegmentTree(const vector<int>& values) {
        n = (int)values.size();
        maximumTree.resize(4 * n);
        gcdTree.resize(4 * n);
        build(1, 0, n - 1, values);
    }

    void assignValue(int index, int value) {
        assignValue(1, 0, n - 1, index, value);
    }

    MaxAnswer rangeMaximum(int left, int right) const {
        return queryMaximum(1, 0, n - 1, left, right);
    }

    int rangeGcd(int left, int right) const {
        return queryGcd(1, 0, n - 1, left, right);
    }
};

// This standard Fenwick variant supports prefix maximum with increases only.
class PrefixMaximumFenwick {
private:
    int n;
    vector<int> tree;

public:
    PrefixMaximumFenwick(const vector<int>& values) {
        n = (int)values.size();
        tree.assign(n + 1, -2147483647);
        for (int i = 0; i < n; i++) increase(i, values[i]);
    }

    void increase(int index, int value) {
        for (int i = index + 1; i <= n; i += i & -i)
            if (value > tree[i]) tree[i] = value;
    }

    int prefixMaximum(int right) const {
        int answer = -2147483647;
        for (int i = right + 1; i > 0; i -= i & -i)
            if (tree[i] > answer) answer = tree[i];
        return answer;
    }
};

int main() {
    int n;
    if (!(cin >> n) || n <= 0) {
        cerr << "Expected a positive array size.\n";
        return 1;
    }
    vector<int> values(n);
    for (int i = 0; i < n; i++) cin >> values[i];
    SegmentTree sensors(values);

    string command;
    while (cin >> command) {
        if (command == "MAX") {
            int left, right;
            cin >> left >> right;
            if (left < 0 || right >= n || left > right) { cerr << "Invalid range.\n"; return 1; }
            MaxAnswer answer = sensors.rangeMaximum(left, right);
            cout << answer.value << " " << answer.count << "\n";
        } else if (command == "GCD") {
            int left, right;
            cin >> left >> right;
            if (left < 0 || right >= n || left > right) { cerr << "Invalid range.\n"; return 1; }
            cout << sensors.rangeGcd(left, right) << "\n";
        } else if (command == "SET") {
            int index, value;
            cin >> index >> value;
            if (index < 0 || index >= n) { cerr << "Invalid index.\n"; return 1; }
            sensors.assignValue(index, value);
        } else {
            cerr << "Unknown command: " << command << "\n";
            return 1;
        }
    }
    return 0;
}
