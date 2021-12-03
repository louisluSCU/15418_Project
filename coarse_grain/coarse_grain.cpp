// C++ program to implement optimized delete in BST (Sequential Base Code from GeeksForGeeks 
// https://www.geeksforgeeks.org/binary-search-tree-set-1-search-and-insertion/?ref=lbp).
#include <bits/stdc++.h>
#include "coarse_grain.h"
using namespace std;

// A utility function to create a new BST node
CGBST::Node* CGBST::newNode(int key)
{
	Node* temp = new Node;
	temp->key = key;
	temp->left = temp->right = NULL;
	return temp;
}

CGBST::CGBST() {
	root = NULL;
}

CGBST::~CGBST() {
	freeTreeHelper(root);
}

// helper function for freeing tree
void CGBST::freeTreeHelper(Node* root) {
	// base case
	if (root == NULL) return;
	freeTreeHelper(root->left);
	freeTreeHelper(root->right);
	delete root;
}

/* Utility function to check if tree contains key
* Concurrent by fine-grained locking */
bool CGBST::contains(int key) {
	/* return false if tree is empty */
	lock.lock();
	if (root == NULL) {
		lock.unlock();
		return false;
	}

	/* Otherwise, iterate down the tree */
	Node *cur = root;
	while (cur) {
		if (cur->key == key) {
			lock.unlock();
			return true;
		} else if (key < cur->key) {
			cur = cur->left;
		} else {
			cur = cur->right;
		}
	}
	lock.unlock();
	return false;
} 

/* A utility function to insert a new node with given key in
* BST (Concurrent by fine-grained locking) */
bool CGBST::add(int key)
{
	/* If the tree is empty, set root to a new node */
	lock.lock();
	if (root == NULL) {
		root = newNode(key);
		lock.unlock();
		return true;
	}

	/* Otherwise, iterate down the tree */
	Node *node = root;
	while (true) {
		if (key == node->key) {
			lock.unlock();
			return false;
		} else if (key < node->key) {
			if (node->left == NULL) {
				node->left = newNode(key);
				break;
			} else {
				node = node->left;
			}
		} else {
			if (node->right == NULL) {
				node->right = newNode(key);
				break;
			} else {
				node = node->right;
			}
		}
	}
	lock.unlock();
	return true;
}

/* Given a binary search tree and a key, this function
deletes the key (Concurrent by fine-grained locking) */
bool CGBST::remove(int key)
{
	// Empty
	lock.lock();
	if (root == NULL) {
		lock.unlock();
		return false;
	}

	// recur down tree until we reach
	// the node to be deleted, locking hand over hand
	Node* prev = NULL;
	Node* cur = root;
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
	if (cur == NULL) {
		lock.unlock();
		return false; // key not found
	}
	// at this point cur is the node to be deleted and prev is its parent, and both cur and prev are locked

	// If one of the children is empty, replace node with non-empty child
	if (cur->left == NULL || cur->right == NULL) {
		Node* temp = (cur->left == NULL) ? cur->right : cur->left;
		if (prev == NULL) {
			// cur is root 
			root = temp;
		} else if (prev->left == cur) {
			prev->left = temp;
		} else {
			prev->right = temp;
		}
		delete cur;
		lock.unlock();
	}

	// If both children exist
	else {
		// TODO: could potentially unlock prev here
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
		lock.unlock();
	}
	return true;
}

