// C++ program to implement optimized delete in BST (Sequential Base Code from GeeksForGeeks 
// https://www.geeksforgeeks.org/binary-search-tree-set-1-search-and-insertion/?ref=lbp).
#include <bits/stdc++.h>
#include "fine_grain/fine_grain.h"
using namespace std;

// A utility function to create a new BST node
FGT_Node* newNode(int item)
{
	FGT_Node* temp = new FGT_Node;
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
// Cited from https://www.geeksforgeeks.org/binary-search-tree-set-1-search-and-insertion/?ref=lbp
FGT_Node* search(FGT_Node* root, int key)
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
// Cited from https://www.geeksforgeeks.org/binary-search-tree-set-1-search-and-insertion/?ref=lbp
void inorder(FGT_Node* root)
{
	if (root != NULL) {
		inorder(root->left);
		printf("%d ", root->key);
		inorder(root->right);
	}
}

// helper function for freeing tree
void freeTreeHelper(FGT_Node* root) {
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

/* Utility function to check if tree contains key
* Concurrent by fine-grained locking */
bool contains(Tree* tree, int key) {
	/* return false if tree is empty */
	tree->lock.lock();
	if (tree->root == NULL) {
		tree->lock.unlock();
		return false;
	}

	/* Otherwise, iterate down the tree */
	FGT_Node *cur = tree->root;
	cur->lock.lock();
	tree->lock.unlock();
	while (cur) {
		if (cur->key == key) {
			cur->lock.unlock();
			return true;
		} else if (key < cur->key) {
			FGT_Node *next = cur->left;
			if (next != NULL) next->lock.lock();
			cur->lock.unlock();
			cur = next;
		} else {
			FGT_Node *next = cur->right;
			if (next != NULL) next->lock.lock();
			cur->lock.unlock();
			cur = next;
		}
	}
	return false;
} 

/* A utility function to insert a new node with given key in
* BST (Concurrent by fine-grained locking) */
void insert(Tree* tree, int key)
{
	/* If the tree is empty, set root to a new node */
	tree->lock.lock();
	if (tree->root == NULL) {
		tree->root = newNode(key);
		tree->lock.unlock();
		return;
	}

	/* Otherwise, iterate down the tree */
	FGT_Node *node = tree->root;
	node->lock.lock();
	tree->lock.unlock();
	while (true) {
		if (key < node->key) {
			if (node->left == NULL) {
				node->left = newNode(key);
				node->lock.unlock();
				break;
			} else {
				FGT_Node *next = node->left;
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
				FGT_Node *next = node->right;
				next->lock.lock();
				node->lock.unlock();
				node = next;
			}
		}
	}
}

/* Given a binary search tree and a key, this function
deletes the key (Concurrent by fine-grained locking) */
void deleteNode(Tree* tree, int key)
{
	// Empty
	tree->lock.lock();
	if (tree->root == NULL) {
		tree->lock.unlock();
		return;
	}

	// recur down tree until we reach
	// the node to be deleted, locking hand over hand
	FGT_Node* prev = NULL;
	FGT_Node* cur = tree->root;
	cur->lock.lock();
	while (cur) {
		if (key < cur->key) {
			if (prev == NULL) tree->lock.unlock();
			else prev->lock.unlock();
			prev = cur;
			cur = cur->left;
			if (cur != NULL) cur->lock.lock();
		} else if (key > cur->key) {
			if (prev == NULL) tree->lock.unlock();
			else prev->lock.unlock();
			prev = cur;
			cur = cur->right;
			if (cur != NULL) cur->lock.lock();
		} else {
			// found key
			break;
		}
	}
	if (cur == NULL) {
		prev->lock.unlock();
		return; // key not found
	}
	// at this point cur is the node to be deleted and prev is its parent, and both cur and prev are locked

	// If one of the children is empty, replace node with non-empty child
	if (cur->left == NULL || cur->right == NULL) {
		FGT_Node* temp = (cur->left == NULL) ? cur->right : cur->left;
		if (prev == NULL) {
			// cur is root 
			tree->root = temp;
		} else if (prev->left == cur) {
			prev->left = temp;
		} else {
			prev->right = temp;
		}
		delete cur;
		if (prev == NULL) tree->lock.unlock();
		else prev->lock.unlock();
	}

	// If both children exist
	else {
		// TODO: could potentially unlock prev here
		FGT_Node* succParent = cur;

		// Find successor
		FGT_Node* succ = cur->right;
		succ->lock.lock();
		while (succ->left != NULL) {
			if (succParent != cur) succParent->lock.unlock();
			succParent = succ;
			succ = succ->left;
			if (succ->left != NULL) succ->left->lock.lock();
		}

		// Delete successor. Since successor
		// is always left child of its parent
		// we can safely make successor's right
		// right child as left of its parent.
		// If there is no succ, then assign
		// succ->right to succParent->right

		// succ and succParent are both locked at this point
		if (succParent != cur)
			succParent->left = succ->right;
		else
			succParent->right = succ->right;

		// Copy Successor Data to root
		cur->key = succ->key;

		// Delete Successor
		delete succ;

		// unlock all 3 (or 2) locks 
		if (succParent != cur) succParent->lock.unlock();
		cur->lock.unlock();
		if (prev == NULL) tree->lock.unlock();
		else prev->lock.unlock();
	}
}

