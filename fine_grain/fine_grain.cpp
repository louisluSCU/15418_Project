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
Node* deleteNode(Node* root, int k)
{
	// Base case
	if (root == NULL)
		return root;

	// Recursive calls for ancestors of
	// node to be deleted
	if (root->key > k) {
		root->left = deleteNode(root->left, k);
		return root;
	}
	else if (root->key < k) {
		root->right = deleteNode(root->right, k);
		return root;
	}

	// We reach here when root is the node
	// to be deleted.

	// If one of the children is empty
	if (root->left == NULL) {
		Node* temp = root->right;
		delete root;
		return temp;
	}
	else if (root->right == NULL) {
		Node* temp = root->left;
		delete root;
		return temp;
	}

	// If both children exist
	else {

		Node* succParent = root;

		// Find successor
		Node* succ = root->right;
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
		if (succParent != root)
			succParent->left = succ->right;
		else
			succParent->right = succ->right;

		// Copy Successor Data to root
		root->key = succ->key;

		// Delete Successor and return root
		delete succ;
		return root;
	}
}

void insertRange(int low, int high, Tree* t) {
	vector<int> v;
	for (int i = low; i < high; i++) {
		v.push_back(i);
	}
	random_shuffle(v.begin(), v.end());
	for (int n: v) {
		insert(t, n);
	}
}

void testConcurrentInsert() {
	Tree* t = newTree();
	insertRange(0, 100000, t);
	for (int i = 0; i < 100000; i++) {
		assert(search(t->root, i) != NULL);
	}
	printf("Sequential insertion passed!\n");
	t = newTree();
	int threadCount = 64;
	vector<thread> tvec;
	for (int i = 0; i < threadCount; i++) {
		tvec.push_back(thread(insertRange, i * 1000, (i+1) * 1000, t));
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
	printf("Concurrent insertion passed!\n");
}

// Driver Code
int main()
{
	testConcurrentInsert();
}

