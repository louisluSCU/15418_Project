#ifndef COARSE_GRAIN_H
#define COARSE_GRAIN_H
#include <bits/stdc++.h>
#include "test/abstract_bst.h"
using namespace std;

class CGBST: public ABST {
	struct Node {
		int key;
		struct Node *left, *right;
	};

	private:
		Node* root;
		mutex lock;
		Node* newNode(int k);
		void freeTreeHelper(Node* root);

	public:
		CGBST();
		~CGBST();
		bool contains(int k);
		bool add(int k);
		bool remove(int k);
};
#endif