#include "lockfree_bst.h"
#include <stdint.h>

#define FLAG_MASK 3UL
#define NULL_MASK 1UL

#define SET_FLAG(op, flag)  (op |= flag)
#define GET_FLAG(op)        ((uintptr_t)op & FLAG_MASK)
#define DE_FLAG(op)         ((Operation*) (((uintptr_t)op >> 2) << 2))

#define SET_NULL(node_ptr)  (node_ptr |= NULL_MASK)
#define IS_NULL(node_ptr)   (((uintptr_t) node_ptr & NULL_MASK) == 1UL)

enum op_flag {
    NONE, TOMB, CHILDCAS, RELOCATE
};

enum find_ret {
    FOUND, NOTFOUND_L, NOTFOUND_R, ABORT
};

int BST::find(int k, Node*& parent, Operation*& parent_op, Node*& curr,
         Operation*& curr_op, Node* root) {
    int result, current_key;
    Node* next, *last_right;
    Operation* last_right_op;

    while (true) {
        result = NOTFOUND_R;
        curr = root;
        curr_op = curr->op;
        if (GET_FLAG(curr_op) != NONE) {
            if (root == &this->root) {
                helpChildCAS(DE_FLAG(curr_op), curr);
                continue;
            }
            else return ABORT;
        }
        next = curr->right;
        last_right = curr;
        last_right_op = curr_op;
        while (!IS_NULL(next)) {
            parent = curr;
            parent_op = curr_op;
            curr = next;
            curr_op = curr->op;
            if (GET_FLAG(curr_op) != NONE) {
                help(parent, parent_op, curr, curr_op);
                continue;
            }
            current_key = curr->key;
            if (k < current_key) {
                result = NOTFOUND_L;
                next = curr->left;
            }
            else if (k > current_key) {
                result = NOTFOUND_R;
                next = curr->right;
                last_right = curr;
                last_right_op = curr_op;
            }
            else {
                result = FOUND;
                return result;
            }
        }

        if (last_right_op == last_right->op || curr_op == curr->op) break;
    }

    return result;
}

bool BST::contains(int k) {

}

bool BST::add(int k) {

}

bool BST::remove(int k) {
    
}