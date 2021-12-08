#include <bits/stdc++.h>
#include "fine_grain/fine_grain.h"
#include "lock_free/lockfree_bst.h"
#include "coarse_grain/coarse_grain.h"
using namespace std;
using namespace std::chrono;

int IS_BST = 1;

FGBST *fgt;
BST *bst;
CGBST *cgbst;

void test_print_tree() {
    if (IS_BST == 1) printf("Lock-Free BST\n");
    else if (IS_BST == 2) printf("Coarse-Grain BST\n");
    else printf("Fine-Grain BST\n");
}

void test_init_tree() {
    if (IS_BST == 1) bst = new BST();
	else if (IS_BST == 2) cgbst = new CGBST();
    else fgt = new FGBST();
}

void test_clear_tree() {
    if (IS_BST == 1) delete bst;
	else if (IS_BST == 2) delete cgbst;
    else delete fgt;
}

bool test_search_tree(int k) {
    if (IS_BST == 1) return bst->contains(k);
	else if (IS_BST == 2) return cgbst->contains(k);
    else return fgt->contains(k);
}

bool test_insert_tree(int k) {
    if (IS_BST == 1) return bst->add(k);
	else if (IS_BST == 2) return cgbst->add(k);
    else return fgt->add(k);
}

bool test_delete_tree(int k) {
    if (IS_BST == 1) return bst->remove(k);
	else if (IS_BST == 2) return cgbst->remove(k);
    else return fgt->remove(k);
}

void insertRange(int low, int high) {
    for (int i = low; i < high; i++) {
        test_insert_tree(i);
    }
}

void deleteRange(int low, int high) {
    for (int i = low; i < high; i++) {
        test_delete_tree(i);
    }
}

void findRange(int low, int high) {
    for (int i = low; i < high; i++) {
        test_search_tree(i);
    }
}

void insertDeleteRange(int low, int high) {
    insertRange(low, high);
    deleteRange(low, high);
}

void insertRangeVec(int low, int high, vector<int> v) {
    for (int i = low; i < high; i++) {
        test_insert_tree(v[i]);
    }
}

void deleteRangeVec(int low, int high, vector<int> v) {
    for (int i = low; i < high; i++) {
        test_delete_tree(v[i]);
    }
}

void findRangeVec(int low, int high, vector<int> v) {
    for (int i = low; i < high; i++) {
        test_search_tree(v[i]);
    }
}

void concurrentInsertRange(int capacity, int numThreads) {
    vector<thread> tvec;
    for (int i = 0; i < numThreads; i++) {
        tvec.push_back(thread(insertRange, i * capacity, (i+1) * capacity));
    }
    for (int i = 0; i < numThreads; i++) {
        tvec[i].join();
    }
}

void concurrentDeleteRange(int capacity, int numThreads) {
    vector<thread> tvec;
    for (int i = 0; i < numThreads; i++) {
        tvec.push_back(thread(insertRange, i * capacity, (i+1) * capacity));
    }
    for (int i = 0; i < numThreads; i++) {
        tvec[i].join();
    }
}

void concurrentFindRange(int capacity, int numThreads) {
    vector<thread> tvec;
    for (int i = 0; i < numThreads; i++) {
        tvec.push_back(thread(findRange, i * capacity, (i+1) * capacity));
    }
    for (int i = 0; i < numThreads; i++) {
        tvec[i].join();
    }
}

void concurrentInsertRangeVec(int capacity, int numThreads, vector<int> &v) {
    vector<thread> tvec;
    for (int i = 0; i < numThreads; i++) {
        tvec.push_back(thread(insertRangeVec, i * capacity, (i+1) * capacity, v));
    }
    for (int i = 0; i < numThreads; i++) {
        tvec[i].join();
    }
}

void concurrentDeleteRangeVec(int capacity, int numThreads, vector<int> &v) {
    vector<thread> tvec;
    for (int i = 0; i < numThreads; i++) {
        tvec.push_back(thread(deleteRangeVec, i * capacity, (i+1) * capacity, v));
    }
    for (int i = 0; i < numThreads; i++) {
        tvec[i].join();
    }
}

void concurrentFindRangeVec(int capacity, int numThreads, vector<int> &v) {
    vector<thread> tvec;
    for (int i = 0; i < numThreads; i++) {
        tvec.push_back(thread(findRangeVec, i * capacity, (i+1) * capacity, v));
    }
    for (int i = 0; i < numThreads; i++) {
        tvec[i].join();
    }
}

vector<int> getRandomRangeVector(int low, int high) {
    vector<int> v;
    for (int i = low; i < high; i++) {
        v.push_back(i);
    }
    random_shuffle(v.begin(), v.end());
    return v;
}

void testInsertDelete(int numThreads, int threadCapacity) {
    test_init_tree();
    auto start = high_resolution_clock::now();
    vector<thread> tvec;
    for (int i = 0; i < numThreads; i++) {
        tvec.push_back(thread(insertDeleteRange, i * threadCapacity, (i+1) * threadCapacity));
    }
    for (int i = 0; i < numThreads; i++) {
        tvec[i].join();
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    printf("InsertDelete for %d capacity and %d threads: %ld milliseconds\n", threadCapacity, numThreads, duration.count());
    test_clear_tree();
}

void testRandomInsert(int numThreads, int threadCapacity) {
    test_init_tree();
    vector<int> v = getRandomRangeVector(0, numThreads * threadCapacity);
    auto start = high_resolution_clock::now();
    concurrentInsertRangeVec(threadCapacity, numThreads, v);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    printf("random insert for %d capacity and %d threads: %ld milliseconds, %ld operations per millisecond\n", threadCapacity, numThreads, duration.count(), threadCapacity * numThreads / max((int64_t)1, duration.count()));
    test_clear_tree();
}

void testImbalanceInsert(int numThreads, int threadCapacity) {
    test_init_tree();
    vector<int> v;
    auto start = high_resolution_clock::now();
    concurrentInsertRange(threadCapacity, numThreads);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    printf("Imbalanced insert for %d capacity and %d threads: %ld milliseconds, %ld operations per millisecond\n", threadCapacity, numThreads, duration.count(), threadCapacity * numThreads / max((int64_t)1, duration.count()));
    test_clear_tree();
}

void testRandomDelete(int numThreads, int threadCapacity) {
    test_init_tree();
    vector<int> v = getRandomRangeVector(0, numThreads * threadCapacity);
    concurrentInsertRangeVec(threadCapacity, numThreads, v);

    // test
    random_shuffle(v.begin(), v.end());
    auto start = high_resolution_clock::now();
    concurrentInsertRangeVec(threadCapacity, numThreads, v);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    printf("Balanced random delete for %d capacity and %d threads: %ld milliseconds, %ld operations per millisecond\n", threadCapacity, numThreads, duration.count(), threadCapacity * numThreads / max((int64_t)1, duration.count()));
    test_clear_tree();
}

void testImbalancRandDelete(int numThreads, int threadCapacity) {
    test_init_tree();
    vector<int> v = getRandomRangeVector(0, numThreads * threadCapacity);
    concurrentInsertRange(threadCapacity, numThreads);

    // test
    auto start = high_resolution_clock::now();
    concurrentDeleteRangeVec(threadCapacity, numThreads, v);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    printf("Imbalanced random delete for %d capacity and %d threads: %ld milliseconds, %ld operations per millisecond\n", threadCapacity, numThreads, duration.count(), threadCapacity * numThreads / max((int64_t)1, duration.count()));
    test_clear_tree();
}

void testRandomFind(int numThreads, int threadCapacity) {
    test_init_tree();
    vector<int> v = getRandomRangeVector(0, numThreads * threadCapacity);
    concurrentInsertRangeVec(threadCapacity, numThreads, v);

    // test
    random_shuffle(v.begin(), v.end());
    auto start = high_resolution_clock::now();
    concurrentFindRangeVec(threadCapacity, numThreads, v);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    printf("Balanced random fine for %d capacity and %d threads: %ld milliseconds, %ld operations per millisecond\n", threadCapacity, numThreads, duration.count(), threadCapacity * numThreads / max((int64_t)1, duration.count()));
    test_clear_tree();
}

void testImbalanceRandFind(int numThreads, int threadCapacity) {
    test_init_tree();
    vector<int> v = getRandomRangeVector(0, numThreads * threadCapacity);

    concurrentInsertRange(threadCapacity, numThreads);

    // test
    auto start = high_resolution_clock::now();
    concurrentFindRangeVec(threadCapacity, numThreads, v);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    printf("Imbalanced random find for %d capacity and %d threads: %ld milliseconds, %ld operations per millisecond\n", threadCapacity, numThreads, duration.count(), threadCapacity * numThreads / max((int64_t)1, duration.count()));
    test_clear_tree();
}

void testImbalanceSeqDelete(int numThreads, int threadCapacity) {
    test_init_tree();
    concurrentInsertRange(threadCapacity, numThreads);

    auto start = high_resolution_clock::now();
    concurrentDeleteRange(threadCapacity, numThreads);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    printf("Imbalanced seq delete for %d capacity and %d threads: %ld milliseconds, %ld operations per millisecond\n", threadCapacity, numThreads, duration.count(), threadCapacity * numThreads / max((int64_t)1, duration.count()));
    test_clear_tree();
}

void testImbalanceSeqFind(int numThreads, int threadCapacity) {
    test_init_tree();
    concurrentInsertRange(threadCapacity, numThreads);

    auto start = high_resolution_clock::now();
    concurrentFindRange(threadCapacity, numThreads);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    printf("Imbalanced seq find for %d capacity and %d threads: %ld milliseconds, %ld operations per millisecond\n", threadCapacity, numThreads, duration.count(), threadCapacity * numThreads / max((int64_t)1, duration.count()));
    test_clear_tree();
}

void testBalancedSeqDelete(int numThreads, int threadCapacity) {
    test_init_tree();
    vector<int> v = getRandomRangeVector(0, numThreads * threadCapacity);
    concurrentInsertRangeVec(threadCapacity, numThreads, v);

    auto start = high_resolution_clock::now();
    concurrentDeleteRange(threadCapacity, numThreads);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    printf("Balanced seq delete for %d capacity and %d threads: %ld milliseconds, %ld operations per millisecond\n", threadCapacity, numThreads, duration.count(), threadCapacity * numThreads / max((int64_t)1, duration.count()));
    test_clear_tree();
}

void testBalancedSeqFind(int numThreads, int threadCapacity) {
    test_init_tree();
    vector<int> v = getRandomRangeVector(0, numThreads * threadCapacity);
    concurrentInsertRangeVec(threadCapacity, numThreads, v);

    auto start = high_resolution_clock::now();
    concurrentFindRange(threadCapacity, numThreads);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    printf("Balanced seq find for %d capacity and %d threads: %ld milliseconds, %ld operations per millisecond\n", threadCapacity, numThreads, duration.count(), threadCapacity * numThreads / max((int64_t)1, duration.count()));
    test_clear_tree();
}

int main(int argc, char const *argv[])
{
    /* code */
    vector<int> numThreads = {1, 4, 16, 64, 128};
    vector<int> capacities = {10000, 1000};
    vector<int> mode = {1, 2, 3};
    for (int m: mode) {
        IS_BST = m;
        test_print_tree();
        for (int capacity: capacities) {
            for (int numThread: numThreads) {
                testRandomInsert(numThread, capacity);
            }
            for (int numThread: numThreads) {
                testRandomDelete(numThread, capacity);
            }
            for (int numThread: numThreads) {
                testRandomFind(numThread, capacity);
            }
        }
    }
    
    
    return 0;
}
