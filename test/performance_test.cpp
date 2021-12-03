#include <bits/stdc++.h>
#include "fine_grain/fine_grain.h"
#include "abstract_bst.h"

using namespace std;
using namespace std::chrono;

void insertRange(ABST* t, int low, int high) {
    for (int i = low; i < high; i++) {
        t->add(i);
    }
}

void deleteRange(ABST* t, int low, int high) {
    for (int i = low; i < high; i++) {
        t->remove(i);
    }
}

void insertDeleteRange(ABST* t, int low, int high) {
    insertRange(t, low, high);
    deleteRange(t, low, high);
}

void testInsertDelete(ABST* t, int numThreads, int threadCapacity) {
    vector<thread> tvec;
    for (int i = 0; i < numThreads; i++) {
        tvec.push_back(thread(insertDeleteRange, t, i * threadCapacity, (i+1) * threadCapacity));
    }
    for (int i = 0; i < numThreads; i++) {
        tvec[i].join();
    }
}

int main(int argc, char const *argv[])
{
    /* code */
    int capacity = 10000;
    vector<int> numThreads = {1, 4, 16, 64, 128};
    for (int numThread: numThreads) {
        FGBST* t = new FGBST();
        auto start = high_resolution_clock::now();
        testInsertDelete(t, numThread, capacity);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stop - start);
        printf("Fine Grained InsertDelete for %d capacity and %d threads: %lld milliseconds\n", capacity, numThread, duration.count());
        delete t;
    }
    return 0;
}
