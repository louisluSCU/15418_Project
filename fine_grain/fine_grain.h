#ifndef FINE_GRAIN_H
#define FINE_GRAIN_H
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

Node* search(Node* root, int key);

void insert(Tree* tree, int key);

void deleteNode(Tree* tree, int key);

bool contains(Tree* tree, int key);

Tree* newTree();

void freeTree(Tree* t);
#endif