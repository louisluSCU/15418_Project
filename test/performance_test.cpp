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

void deleteRange(int low, int high) {
    for (int i = low; i < high; i++) {
        test_delete_tree(i);
    }
}

void insertDeleteRange(int low, int high) {
    insertRange(low, high);
    deleteRange(low, high);
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
    printf("InsertDelete for %d capacity and %d threads: %lld milliseconds\n", threadCapacity, numThreads, duration.count());
    test_clear_tree();
}

void testRandomInsert(int numThreads, int threadCapacity) {
    test_init_tree();
    vector<int> v;
    vector<thread> tvec;
    for (int i = 0; i < numThreads * threadCapacity; i++) {
        v.push_back(i);
    }
    auto start = high_resolution_clock::now();
    random_shuffle(v.begin(), v.end());
    for (int i = 0; i < numThreads; i++) {
        tvec.push_back(thread(insertRangeVec, i * threadCapacity, (i+1) * threadCapacity, v));
    }
    for (int i = 0; i < numThreads; i++) {
        tvec[i].join();
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    printf("random insert for %d capacity and %d threads: %lld milliseconds\n", threadCapacity, numThreads, duration.count());
    test_clear_tree();
}

void testRandomDelete(int numThreads, int threadCapacity) {
    test_init_tree();
    vector<int> v;
    vector<thread> tvec;
    for (int i = 0; i < numThreads * threadCapacity; i++) {
        v.push_back(i);
    }
    
    // insert using all threads to reduce test time
    random_shuffle(v.begin(), v.end());
    for (int i = 0; i < numThreads; i++) {
        tvec.push_back(thread(insertRangeVec, i * threadCapacity, (i+1) * threadCapacity, v));
    }
    for (int i = 0; i < numThreads; i++) {
        tvec[i].join();
    }
    tvec.clear();
    // shuffle again so we don't delete in the exact same order we insert
    random_shuffle(v.begin(), v.end());

    // test
    auto start = high_resolution_clock::now();
    for (int i = 0; i < numThreads; i++) {
        tvec.push_back(thread(deleteRangeVec, i * threadCapacity, (i+1) * threadCapacity, v));
    }
    for (int i = 0; i < numThreads; i++) {
        tvec[i].join();
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    printf("random delete for %d capacity and %d threads: %lld milliseconds\n", threadCapacity, numThreads, duration.count());
    test_clear_tree();
}

void testRandomFind(int numThreads, int threadCapacity) {
    test_init_tree();
    vector<int> v;
    vector<thread> tvec;
    for (int i = 0; i < numThreads * threadCapacity; i++) {
        v.push_back(i);
    }
    
    // insert using all threads to reduce test time
    random_shuffle(v.begin(), v.end());
    for (int i = 0; i < numThreads; i++) {
        tvec.push_back(thread(insertRangeVec, i * threadCapacity, (i+1) * threadCapacity, v));
    }
    for (int i = 0; i < numThreads; i++) {
        tvec[i].join();
    }
    tvec.clear();
    // shuffle again so we don't delete in the exact same order we insert
    random_shuffle(v.begin(), v.end());

    // test
    auto start = high_resolution_clock::now();
    for (int i = 0; i < numThreads; i++) {
        tvec.push_back(thread(findRangeVec, i * threadCapacity, (i+1) * threadCapacity, v));
    }
    for (int i = 0; i < numThreads; i++) {
        tvec[i].join();
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    printf("random find for %d capacity and %d threads: %lld milliseconds\n", threadCapacity, numThreads, duration.count());
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
