#ifndef FINE_GRAIN_H
#define FINE_GRAIN_H
#include <bits/stdc++.h>
using namespace std;

struct FGT_Node {
	int key;
	struct FGT_Node *left, *right;
	mutex lock;
};

struct Tree {
	struct FGT_Node *root;
	mutex lock;
};

FGT_Node* search(FGT_Node* root, int key);

void insert(Tree* tree, int key);

void deleteNode(Tree* tree, int key);

bool contains(Tree* tree, int key);

Tree* newTree();

void freeTree(Tree* t);
#endif