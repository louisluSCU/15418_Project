// (Sequential Base Code from GeeksForGeeks 
// https://www.geeksforgeeks.org/binary-search-tree-set-1-search-and-insertion/?ref=lbp).
#include <bits/stdc++.h>
#include "fine_grain.h"
using namespace std;

// A utility function to create a new BST node
FGBST::Node* FGBST::newNode(int key)
{
	Node* temp = new Node;
	temp->key = key;
	temp->left = temp->right = NULL;
	return temp;
}

FGBST::FGBST() {
	root = NULL;
}

FGBST::~FGBST() {
	freeTreeHelper(root);
}

// helper function for freeing tree
void FGBST::freeTreeHelper(Node* root) {
	// base case
	if (root == NULL) return;
	freeTreeHelper(root->left);
	freeTreeHelper(root->right);
	delete root;
}

/* Utility function to check if tree contains key
* Concurrent by fine-grained locking */
bool FGBST::contains(int key) {
	/* return false if tree is empty */
	lock.lock();
	if (root == NULL) {
		lock.unlock();
		return false;
	}

	/* Otherwise, iterate down the tree */
	Node *cur = root;
	cur->lock.lock();
	lock.unlock();
	while (cur) {
		if (cur->key == key) {
			cur->lock.unlock();
			return true;
		} else if (key < cur->key) {
			Node *next = cur->left;
			if (next != NULL) next->lock.lock();
			cur->lock.unlock();
			cur = next;
		} else {
			Node *next = cur->right;
			if (next != NULL) next->lock.lock();
			cur->lock.unlock();
			cur = next;
		}
	}
	return false;
} 

/* A utility function to insert a new node with given key in
* BST (Concurrent by fine-grained locking) */
bool FGBST::add(int key)
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
	node->lock.lock();
	lock.unlock();
	while (true) {
		if (key == node->key) {
			// key already exists
			node->lock.unlock();
			return false;
		} else if (key < node->key) {
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
	return true;
}

/* Given a binary search tree and a key, this function
deletes the key (Concurrent by fine-grained locking) */
bool FGBST::remove(int key)
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
	cur->lock.lock();
	while (cur) {
		if (key < cur->key) {
			if (prev == NULL) lock.unlock();
			else prev->lock.unlock();
			prev = cur;
			cur = cur->left;
			if (cur != NULL) cur->lock.lock();
		} else if (key > cur->key) {
			if (prev == NULL) lock.unlock();
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
		if (prev == NULL) lock.unlock();
		else prev->lock.unlock();
	}

	// If both children exist
	else {
		Node* succParent = cur;

		// Find successor
		Node* succ = cur->right;
		succ->lock.lock();
		while (succ->left != NULL) {
			if (succParent != cur) succParent->lock.unlock();
			succParent = succ;
			succ = succ->left;
			succ->lock.lock();
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
		if (prev == NULL) lock.unlock();
		else prev->lock.unlock();
	}
	return true;
}

