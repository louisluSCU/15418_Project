#include <bits/stdc++.h>
#include <algorithm>
#include <random>

#include "fine_grain/fine_grain.h"
#include "lock_free/lockfree_bst.h"
#include "coarse_grain/coarse_grain.h"
using namespace std;
using namespace std::chrono;

#define IS_BST 1

FGBST *fgt;
BST *bst;
CGBST *cgbst;

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

void insertRangeIdx(int low, int high, vector<int> nums) {
    for (int i = low; i < high; i++) {
        test_insert_tree(nums[i]);
    }
}

void deleteRangeIdx(int low, int high, vector<int> nums) {
    for (int i = low; i < high; i++) {
        test_delete_tree(nums[i]);
    }
}

void insertDeleteRange(int low, int high) {
    insertRange(low, high);
    deleteRange(low, high);
}

void insertDeleteBatch(int low, int high, int batchSize) {
    int numBatch = (high - low) / batchSize;
    if ((high - low) % batchSize != 0) numBatch += 1;

    for (int i = 0; i < numBatch; i++) {
        int endIdx = low + (i + 1) * batchSize;
        if (endIdx > high) {
            endIdx = high;
        }
        insertRange(low + i * batchSize, endIdx);
        deleteRange(low + i * batchSize, endIdx);
    }
}

void insertDeleteRandomRange(int low, int high, vector<int> nums) {
    insertRangeIdx(low, high, nums);
    deleteRangeIdx(low, high, nums);
}

void testInsertDeleteBatch(int numThreads, int threadCapacity, int batchSize) {
    vector<thread> tvec;
    for (int i = 0; i < numThreads; i++) {
        tvec.push_back(thread(insertDeleteBatch, i * threadCapacity, (i+1) * threadCapacity, batchSize));
    }
    for (int i = 0; i < numThreads; i++) {
        tvec[i].join();
    }
}

void testInsertDeleteRange(int numThreads, int threadCapacity) {
    vector<thread> tvec;
    for (int i = 0; i < numThreads; i++) {
        tvec.push_back(thread(insertDeleteRange, i * threadCapacity, (i+1) * threadCapacity));
    }
    for (int i = 0; i < numThreads; i++) {
        tvec[i].join();
    }
}

void testInsertDeleteRandomRange(int numThreads, int threadCapacity) {
    vector<thread> tvec;
    vector<int> nums;
    for (int i = 0; i < numThreads * threadCapacity; i++) {
        nums.push_back(i);
    }
    auto rng = std::default_random_engine {};
    std::shuffle(nums.begin(), nums.end(), rng);

    for (int i = 0; i < numThreads; i++) {
        tvec.push_back(thread(insertDeleteRandomRange, i * threadCapacity, (i+1) * threadCapacity, nums));
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

    // insert delete in range
    for (int numThread: numThreads) {
        test_init_tree();
        auto start = high_resolution_clock::now();
        testInsertDeleteRange(numThread, capacity);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stop - start);
        printf("InsertDeleteRange for %d capacity and %d threads: %ld milliseconds\n", capacity, numThread, duration.count());
        test_clear_tree();
    }

    // insert delete in batch
    for (int numThread: numThreads) {
        test_init_tree();
        auto start = high_resolution_clock::now();
        testInsertDeleteBatch(numThread, capacity, 100);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stop - start);
        printf("InsertDeleteBatch for %d capacity and %d threads: %ld milliseconds\n", capacity, numThread, duration.count());
        test_clear_tree();
    }

    // random insert delete
    for (int numThread: numThreads) {
        test_init_tree();
        auto start = high_resolution_clock::now();
        testInsertDeleteRandomRange(numThread, capacity);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stop - start);
        printf("InsertDeleteRandom for %d capacity and %d threads: %ld milliseconds\n", capacity, numThread, duration.count());
        test_clear_tree();
    }
    return 0;
}
