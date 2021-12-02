#include <fine_grain.h>
using namespace std;

// insert the range in random order
void insertRangeRandom(int low, int high, Tree* t) {
	vector<int> v;
	for (int i = low; i < high; i++) {
		v.push_back(i);
	}
	random_shuffle(v.begin(), v.end());
	for (int n: v) {
		insert(t, n);
	}
}

void testSequentialInsert() {
	Tree* t = newTree();
	insertRangeRandom(0, 100000, t);
	for (int i = 0; i < 100000; i++) {
		assert(search(t->root, i) != NULL);
	}
	freeTree(t);
	printf("Sequential insertion passed!\n");
}

void testConcurrentInsert() {
	Tree* t = newTree();

	int threadCount = 64;
	vector<thread> tvec;
	for (int i = 0; i < threadCount; i++) {
		tvec.push_back(thread(insertRangeRandom, i * 1000, (i+1) * 1000, t));
	}
	for (int i = 0; i < threadCount; i++) {
		tvec[i].join();
	}
	for (int i = 0; i < 1000 * threadCount; i++) {
		if (search(t->root, i) == NULL) {
			printf("Concurrent insertion failed: %d is lost\n", i);
			return;
		}
	}
	freeTree(t);
	printf("Concurrent insertion passed!\n");
}

// delete even numbers in range
void deleteRangeEven(int low, int high, Tree *t) {
	int start = (low % 2 == 0) ? low : low + 1;
	for (int i = start; i < high; i += 2) {
		deleteNode(t, i);
	}
}

// delete entire range
void deleteRange(int low, int high, Tree *t) {
	for (int i = low; i < high; i++) {
		deleteNode(t, i);
	}
}

// insert range in random order than delete even elements
void insertRangeDeleteEven(int low, int high, int interval, Tree *t) {
	for (int i = low; i < high; i += interval) {
		insertRangeRandom(i, min(high, i + interval), t);
		deleteRangeEven(i, min(high, i + interval), t);
	}
}

// insert range in random order, delete even elements and search for elements
void insertRangeDeleteEvenSearch(int low, int high, int interval, Tree *t, bool *b) {
	for (int i = low; i < high; i += interval) {
		insertRangeRandom(i, min(high, i + interval), t);
		deleteRangeEven(i, min(high, i + interval), t);
		for (int j = i; j < min(high, i + interval); j++) {
			if (j % 2 == 0 && contains(t, j)) {
				printf("insertDeleteSearch failed: %d should be deleted but is found\n", j);
				*b = false;
				return;
			} else if (j % 2 == 1 && !contains(t, j)) {
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

	Tree *t = newTree();
	int totalSize = numThreads * threadSize;
	insertRangeRandom(0, totalSize, t);
	// delete all but the smallest number of each thread
	for (int i = 0; i < numThreads; i++) {
		deleteRange(i * threadSize + 1, (i+1) * threadSize, t);
	}
	for (int i = 0; i < totalSize; i++) {
		if (i % threadSize == 0) {
			assert(search(t->root, i) != NULL);
		} else {
			assert(search(t->root, i) == NULL);
		}
	}
	freeTree(t);
	printf("Sequential deletion passed!\n");
}

void testConcurrentDelete() {
	int numThreads = 128;
	int threadSize = 1000;

	Tree *t = newTree();
	
	insertRangeRandom(0, numThreads * threadSize, t);
	vector<thread> tvec;
	for (int i = 0; i < numThreads; i++) {
		tvec.push_back(thread(deleteRange, i * threadSize + 1, (i+1) * threadSize, t));
	}
	for (int i = 0; i < numThreads; i++) {
		tvec[i].join();
	}
	for (int i = 0; i < numThreads * threadSize; i++) {
		if (i % threadSize == 0) {
			if (search(t->root, i) == NULL) {
				printf("Concurrent deletion failed: %d should NOT be deleted but dropped\n", i);
				return;
			}
		} else {
			if (search(t->root, i) != NULL) {
				printf("Concurrent deletion failed: %d should be deleted but is kept\n", i);
				return;
			}
		}	
	}
	freeTree(t);
	printf("Concurrent deletion passed!\n");
}

void testMixInsertDelete() {
	Tree* t = newTree();
	int numThreads = 64;
	int threadSize = 1000;
	int interval = 1;
	vector<thread> tvec;
	for (int i = 0; i < numThreads; i++) {
		tvec.push_back(thread(insertRangeDeleteEven, i * threadSize, (i+1) * threadSize, interval, t));
	}
	for (int i = 0; i < numThreads; i++) {
		tvec[i].join();
	}
	for (int i = 0; i < numThreads * threadSize; i += 2) {
		if (search(t->root, i) != NULL) {
			printf("mix failed: %d should be deleted but is kept\n", i);
			freeTree(t);
			return;
		}
	}
	for (int i = 1; i < numThreads * threadSize; i += 2) {
		if (search(t->root, i) == NULL) {
			printf("mix failed: %d should be kept but is lost\n", i);
			freeTree(t);
			return;
		}
	}
	freeTree(t);
	printf("insert delete test passed!\n");
}

void testInsertDeleteSearchMix() {
	Tree* t = newTree();
	int numThreads = 64;
	int threadSize = 1000;
	int interval = 1;
	bool result[numThreads];
	vector<thread> tvec;
	for (int i = 0; i < numThreads; i++) {
		tvec.push_back(thread(insertRangeDeleteEvenSearch, i * threadSize, (i+1) * threadSize, interval, t, &result[i]));
	}
	for (int i = 0; i < numThreads; i++) {
		tvec[i].join();
	}
	freeTree(t);
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

