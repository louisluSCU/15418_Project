#include <bits/stdc++.h>
#include <fine_grain.h>
using namespace std;
using namespace std::chrono;

void insertRange(Tree* t, int low, int high) {
    for (int i = low; i < high; i++) {
        insert(t, i);
    }
}

void deleteRange(Tree* t, int low, int high) {
    for (int i = low; i < high; i++) {
        deleteNode(t, i);
    }
}

void insertDeleteRange(Tree* t, int low, int high) {
    insertRange(t, low, high);
    deleteRange(t, low, high);
}

void testInsertDelete(Tree* t, int numThreads, int threadCapacity) {
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
        Tree* t = newTree();
        auto start = high_resolution_clock::now();
        testInsertDelete(t, numThread, capacity);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stop - start);
        printf("Fine Grained InsertDelete for %d capacity and %d threads: %lld milliseconds\n", capacity, numThread, duration.count());
        freeTree(t);
        delete t;
    }
    return 0;
}
