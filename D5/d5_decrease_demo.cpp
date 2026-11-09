// D5: Repeatedly lower the current maximum and rebuild a prefix-max Fenwick tree.
// This intentionally simple baseline shows why decreases are expensive here.
#include <iostream>
#include <vector>
#include <cstdlib>

using namespace std;

class PrefixMaxFenwick {
private:
    int n;
    vector<int> bit;

public:
    PrefixMaxFenwick(const vector<int>& values) {
        n = (int)values.size();
        bit.assign(n + 1, -2147483647);
        // Simple construction: insert each array value.
        for (int i = 0; i < n; i++) increase(i, values[i]);
    }

    void increase(int index, int value) {
        for (int i = index + 1; i <= n; i += i & -i)
            if (value > bit[i]) bit[i] = value;
    }

    int prefixMaximum(int right) const {
        int answer = -2147483647;
        for (int i = right + 1; i > 0; i -= i & -i)
            if (bit[i] > answer) answer = bit[i];
        return answer;
    }
};

int main() {
    int n, rounds;
    cout << "Enter array size and number of decreases: ";
    if (!(cin >> n >> rounds) || n <= 0 || rounds < 0 || rounds > n) {
        cerr << "Use a positive size and between 0 and n decreases.\n";
        return 1;
    }

    vector<int> values(n);
    for (int i = 0; i < n; i++) values[i] = n - i;
    long long rebuildInsertions = 0;

    for (int step = 1; step <= rounds; step++) {
        // Search for the current maximum, then lower it below every other value.
        int maximumIndex = 0;
        for (int i = 1; i < n; i++)
            if (values[i] > values[maximumIndex]) maximumIndex = i;
        if (n == 1) values[maximumIndex]--;
        else {
            int minimumOther = maximumIndex == 0 ? 1 : 0;
            for (int i = 0; i < n; i++)
                if (i != maximumIndex && values[i] < values[minimumOther]) minimumOther = i;
            values[maximumIndex] = values[minimumOther] - 1;
        }

        // An old maximum cannot be removed from max-only Fenwick cells.
        // Rebuild from the changed array to restore correct answers.
        PrefixMaxFenwick rebuilt(values);
        rebuildInsertions += n;

        int trueMaximum = values[0];
        for (int i = 1; i < n; i++)
            if (values[i] > trueMaximum) trueMaximum = values[i];
        cout << "decrease " << step
             << ": current whole-array maximum = " << rebuilt.prefixMaximum(n - 1)
             << " (direct scan = " << trueMaximum << ")\n";
    }

    cout << "Array values processed during rebuilds: " << rebuildInsertions << "\n";
    cout << "With this constructor, each rebuild uses n Fenwick insertions; "
            "the total work grows with n times the number of decreases.\n";
    return 0;
}
