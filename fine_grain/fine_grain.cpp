// C++ program to implement optimized delete in BST (Sequential Base Code from GeeksForGeeks 
// https://www.geeksforgeeks.org/binary-search-tree-set-1-search-and-insertion/?ref=lbp).
#include <bits/stdc++.h>
using namespace std;

struct Node {
	int key;
	struct Node *left, *right;
	mutex lock;
};

struct Tree {
	struct Node *root;
	mutex lock;
};

// A utility function to create a new BST node
Node* newNode(int item)
{
	Node* temp = new Node;
	temp->key = item;
	temp->left = temp->right = NULL;
	return temp;
}

Tree* newTree() {
	Tree* t = new Tree;
	t->root = NULL;
	return t;
}

// C function to search a given key in a given BST
Node* search(Node* root, int key)
{
    // Base Cases: root is null or key is present at root
    if (root == NULL || root->key == key)
       return root;
    
    // Key is greater than root's key
    if (root->key < key)
       return search(root->right, key);
 
    // Key is smaller than root's key
    return search(root->left, key);
}

// A utility function to do inorder traversal of BST
void inorder(Node* root)
{
	if (root != NULL) {
		inorder(root->left);
		printf("%d ", root->key);
		inorder(root->right);
	}
}

// helper function for freeing tree
void freeTreeHelper(Node* root) {
	// base case
	if (root == NULL) return;
	freeTreeHelper(root->left);
	freeTreeHelper(root->right);
	delete root;
}

// utility function for deleting entire tree and freeing all memory
void freeTree(Tree* tree) {
	freeTreeHelper(tree->root);
	tree->root = NULL;
}

/* A utility function to insert a new node with given key in
* BST */
void insert(Tree* tree, int key)
{
	/* If the tree is empty, set root to a new node */
	tree->lock.lock();
	if (tree->root == NULL) {
		tree->root = newNode(key);
		tree->lock.unlock();
		return;
	}

	/* Otherwise, recur down the tree */
	Node *node = tree->root;
	node->lock.lock();
	tree->lock.unlock();
	while (true) {
		if (key < node->key) {
			if (node->left == NULL) {
				node->left = newNode(key);
				node->lock.unlock();
				break;
			} else {
				Node *next = node->left;
				next->lock.lock();
				node->lock.unlock();
				node = next;
			}
		} else {
			if (node->right == NULL) {
				node->right = newNode(key);
				node->lock.unlock();
				break;
			} else {
				Node *next = node->right;
				next->lock.lock();
				node->lock.unlock();
				node = next;
			}
		}
	}
}

/* Given a binary search tree and a key, this function
deletes the key and returns the new root */
void deleteNode(Tree* tree, int key)
{
	// Empty
	if (tree->root == NULL)
		return;

	// recur down tree until we reach
	// the node to be deleted
	Node* prev = NULL;
	Node* cur = tree->root;
	while (cur) {
		if (key < cur->key) {
			prev = cur;
			cur = cur->left;
		} else if (key > cur->key) {
			prev = cur;
			cur = cur->right;
		} else {
			// found key
			break;
		}
	}
	if (cur == NULL) return; // key not found

	// at this point cur is the node to be deleted and prev is its parent

	// If one of the children is empty, replace node with non-empty child
	if (cur->left == NULL || cur->right == NULL) {
		Node* temp = (cur->left == NULL) ? cur->right : cur->left;
		if (prev == NULL) {
			// cur is root 
			tree->root = temp;
		} else if (prev->left == cur) {
			prev->left = temp;
		} else {
			prev->right = temp;
		}
		delete cur;
	}

	// If both children exist
	else {

		Node* succParent = cur;

		// Find successor
		Node* succ = cur->right;
		while (succ->left != NULL) {
			succParent = succ;
			succ = succ->left;
		}

		// Delete successor. Since successor
		// is always left child of its parent
		// we can safely make successor's right
		// right child as left of its parent.
		// If there is no succ, then assign
		// succ->right to succParent->right
		if (succParent != cur)
			succParent->left = succ->right;
		else
			succParent->right = succ->right;

		// Copy Successor Data to root
		cur->key = succ->key;

		// Delete Successor
		delete succ;
	}
}

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
		deleteRangeEven(i, i + interval, t);
	}
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
}

