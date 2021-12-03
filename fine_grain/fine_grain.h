#ifndef FINE_GRAIN_H
#define FINE_GRAIN_H
#include <bits/stdc++.h>
using namespace std;

class FGBST {
	struct Node {
		int key;
		struct Node *left, *right;
		mutex lock;
	};

	private:
		Node* root;
		mutex lock;
		Node* newNode(int k);
		void freeTreeHelper(Node* root);

	public:
		FGBST();
		~FGBST();
		bool contains(int k);
		bool add(int k);
		bool remove(int k);
};
#endif