#include "fine_grain/fine_grain.h"
#include "lock_free/lockfree_bst.h"
#include "coarse_grain/coarse_grain.h"

using namespace std;

#define IS_BST 2

Tree *fgt;
BST *bst;
CGBST *cgbst;

void test_init_tree() {
    if (IS_BST == 1) bst = new BST();
	else if (IS_BST == 2) cgbst = new CGBST();
    else fgt = newTree();
}

void test_clear_tree() {
    if (IS_BST == 1) delete bst;
	else if (IS_BST == 2) delete cgbst;
    else freeTree(fgt);
}

bool test_search_tree(int k) {
    if (IS_BST == 1) return bst->contains(k);
	else if (IS_BST == 2) return cgbst->contains(k);
    else return search(fgt->root, k) != NULL;
}

bool test_insert_tree(int k) {
    if (IS_BST == 1) return bst->add(k);
	else if (IS_BST == 2) return cgbst->add(k);
    else {
        insert(fgt, k);
        return true;
    }
}

bool test_delete_tree(int k) {
    if (IS_BST == 1) return bst->remove(k);
	else if (IS_BST == 2) return cgbst->remove(k);
    else {
        deleteNode(fgt, k);
        return true;
    }
}

// insert the range in random order
void insertRangeRandom(int low, int high) {
	vector<int> v;
	for (int i = low; i < high; i++) {
		v.push_back(i);
	}
	random_shuffle(v.begin(), v.end());
	for (int n: v) {
		test_insert_tree(n);
	}
}

void testSequentialInsert() {
	test_init_tree();
	insertRangeRandom(0, 100000);
	for (int i = 0; i < 100000; i++) {
		assert(test_search_tree(i));
	}
	test_clear_tree();
	printf("Sequential insertion passed!\n");
}

void testConcurrentInsert() {
    test_init_tree();

	int threadCount = 64;
	vector<thread> tvec;
	for (int i = 0; i < threadCount; i++) {
		tvec.push_back(thread(insertRangeRandom, i * 1000, (i+1) * 1000));
	}
	for (int i = 0; i < threadCount; i++) {
		tvec[i].join();
	}
	for (int i = 0; i < 1000 * threadCount; i++) {
		if (!test_search_tree(i)) {
			printf("Concurrent insertion failed: %d is lost\n", i);
			return;
		}
	}
    test_clear_tree();
	printf("Concurrent insertion passed!\n");
}

// delete even numbers in range
void deleteRangeEven(int low, int high) {
	int start = (low % 2 == 0) ? low : low + 1;
	for (int i = start; i < high; i += 2) {
		test_delete_tree(i);
	}
}

// delete entire range
void deleteRange(int low, int high) {
	for (int i = low; i < high; i++) {
		test_delete_tree(i);
	}
}

// insert range in random order than delete even elements
void insertRangeDeleteEven(int low, int high, int interval) {
	for (int i = low; i < high; i += interval) {
		insertRangeRandom(i, min(high, i + interval));
		deleteRangeEven(i, min(high, i + interval));
	}
}

// insert range in random order, delete even elements and search for elements
void insertRangeDeleteEvenSearch(int low, int high, int interval, bool *b) {
	for (int i = low; i < high; i += interval) {
		insertRangeRandom(i, min(high, i + interval));
		deleteRangeEven(i, min(high, i + interval));
		for (int j = i; j < min(high, i + interval); j++) {
			if (j % 2 == 0 && test_search_tree(j)) {
				printf("insertDeleteSearch failed: %d should be deleted but is found\n", j);
				*b = false;
				return;
			} else if (j % 2 == 1 && !test_search_tree(j)) {
				printf("insertDeleteSearch failed: %d should be kept but is not found\n", j);
				*b = false;
				return;
			}
		}
	}
	*b = true;
}

void testSequentialDelete() {
	int numThreads = 128;
	int threadSize = 1000;

    test_init_tree();
	int totalSize = numThreads * threadSize;
	insertRangeRandom(0, totalSize);
	// delete all but the smallest number of each thread
	for (int i = 0; i < numThreads; i++) {
		deleteRange(i * threadSize + 1, (i+1) * threadSize);
	}
	for (int i = 0; i < totalSize; i++) {
		if (i % threadSize == 0) {
			assert(test_search_tree(i));
		} else {
			assert(!test_search_tree(i));
		}
	}
    test_clear_tree();
	printf("Sequential deletion passed!\n");
}

void testConcurrentDelete() {
	int numThreads = 128;
	int threadSize = 1000;

	test_init_tree();
	
	insertRangeRandom(0, numThreads * threadSize);
	vector<thread> tvec;
	for (int i = 0; i < numThreads; i++) {
		tvec.push_back(thread(deleteRange, i * threadSize + 1, (i+1) * threadSize));
	}
	for (int i = 0; i < numThreads; i++) {
		tvec[i].join();
	}
	for (int i = 0; i < numThreads * threadSize; i++) {
		if (i % threadSize == 0) {
			if (!test_search_tree(i)) {
				printf("Concurrent deletion failed: %d should NOT be deleted but dropped\n", i);
				return;
			}
		} else {
			if (test_search_tree(i)) {
				printf("Concurrent deletion failed: %d should be deleted but is kept\n", i);
				return;
			}
		}	
	}
	test_clear_tree();
	printf("Concurrent deletion passed!\n");
}

void testMixInsertDelete() {
	test_init_tree();
	int numThreads = 64;
	int threadSize = 1000;
	int interval = 1;
	vector<thread> tvec;

	for (int i = 0; i < numThreads; i++) {
		tvec.push_back(thread(insertRangeDeleteEven, i * threadSize, (i+1) * threadSize, interval));
	}
	for (int i = 0; i < numThreads; i++) {
		tvec[i].join();
	}
	for (int i = 0; i < numThreads * threadSize; i += 2) {
		if (test_search_tree(i)) {
			printf("mix failed: %d should be deleted but is kept\n", i);
			test_clear_tree();
			return;
		}
	}
	for (int i = 1; i < numThreads * threadSize; i += 2) {
		if (!test_search_tree(i)) {
			printf("mix failed: %d should be kept but is lost\n", i);
			test_clear_tree();
			return;
		}
	}
	test_clear_tree();
	printf("insert delete test passed!\n");
}

void testInsertDeleteSearchMix() {
	test_init_tree();
	int numThreads = 64;
	int threadSize = 1000;
	int interval = 1;
	bool result[numThreads];
	vector<thread> tvec;

	for (int i = 0; i < numThreads; i++) {
		tvec.push_back(thread(insertRangeDeleteEvenSearch, i * threadSize, (i+1) * threadSize, interval, &result[i]));
	}
	for (int i = 0; i < numThreads; i++) {
		tvec[i].join();
	}
	test_clear_tree();
	for (int i = 0; i < numThreads; i++) {
		if (result[i] == false) {
			printf("insertDeleteSearch Failed on thread %d\n", i);
			return;
		}
	}
	printf("insertDeleteSearch passed!\n");
}

// Driver Code
int main()
{
	testSequentialInsert();
	testSequentialDelete();
	for (int i = 0; i < 10; i++) {
		testConcurrentInsert();
	}
	for (int i = 0; i < 10; i++) {
		testConcurrentDelete();
	}
	for (int i = 0; i < 10; i++) {
		testMixInsertDelete();
	}
	for (int i = 0; i < 10; i++) {
		testInsertDeleteSearchMix();
	}
}

