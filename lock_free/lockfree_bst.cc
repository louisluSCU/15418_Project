#include "lockfree_bst.h"
#include <stdint.h>

#define FLAG_MASK 3UL
#define NULL_MASK 1UL

#define SET_FLAG(op, flag)  ((Operation*) ((uintptr_t)op |= flag))
#define GET_FLAG(op)        ((uintptr_t)op & FLAG_MASK)
#define DE_FLAG(op)         ((Operation*) (((uintptr_t)op >> 2) << 2))

#define SET_NULL(node)  ((Node*) ((uintptr_t) node | NULL_MASK))
#define IS_NULL(node)   ((((uintptr_t) node & NULL_MASK) == 1UL)

#define CAS(obj, expected, desired) true

enum op_flag {
    NONE, TOMB, CHILDCAS, RELOCATE
};

enum find_ret {
    FOUND, NOTFOUND_L, NOTFOUND_R, ABORT
};

Node::Node(int k) : key(k) {}

ChildCASOp::ChildCASOp(bool is_left, Node* old, Node* new_n) {
    this->is_left = is_left;
    expected = old;
    update = new_n;
}

RelocateOp::RelocateOp(Node* curr, Operation* curr_op, int curr_key, int new_key) {
    dest = curr;
    dest_op = curr_op;
    key_to_remove = curr_key;
    key_to_put = new_key;
}

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

        // Start traversal process
        while (!IS_NULL(next)) {
            parent = curr;
            parent_op = curr_op;
            curr = next;
            curr_op = next->op;

            // Current search node is occupied
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

        // When target not found
        if (last_right_op == last_right->op || curr_op == curr->op)
            break;
    }

    return result;
}

bool BST::contains(int k) {
    Node* parent, *curr;
    Operation* parent_op, *curr_op;
    return find(k, parent, parent_op, curr, curr_op, &root) == FOUND;
}

bool BST::add(int k) {
    Node* parent, *curr, *new_node;
    Operation* parent_op, *curr_op, *cas_op;
    int result;

    while(true) {
        result = find(k, parent, parent_op, curr, curr_op, &root);
        if (result == FOUND)
            return false;
        new_node = new Node(k);
        bool isLeft = (result == NOTFOUND_L);
        Node* old = isLeft ? curr->left : curr->right;
        cas_op = new ChildCASOp(isLeft, old, new_node);

        // Update target parent's op field and do update
        if (CAS(&curr->op, curr_op, SET_FLAG(cas_op, CHILDCAS))) {
            helpChildCAS(cas_op, curr);
            return true;
        }
    }
}

bool BST::remove(int k) {
    Node* parent, *curr, *replace;
    Operation* parent_op, *curr_op, *replace_op, *relocate_op;

    while (true) {
        if (find(k, parent, parent_op, curr, curr_op, &root) != FOUND)
            return false;

        // Target node has less than two child
        if (IS_NULL(curr->left) || IS_NULL(curr->right)) {
            if (CAS(&curr->op, curr_op, SET_FLAG(curr_op, TOMB))) {
                helpMarked(parent, parent_op, curr);
                return true;
            }
        }
        // Target node has two children
        else {
            if (find(k, parent, parent_op, replace, replace_op, curr) == ABORT || (curr->op != curr_op))
                continue;
            relocate_op = new RelocateOp(curr, curr_op, k, replace->key);
            if (CAS(&replace_op, replace_op, SET_FLAG(relocate_op, RELOCATE))) {
                if (helpRelocate(relocate_op, parent, parent_op, replace))
                    return true;
            }
        }
    }
}

void BST::help(Node* parent, Operation* parent_op, Node* curr, Operation* curr_op) {
    if (GET_FLAG(curr_op) == CHILDCAS)
        helpChildCAS(DE_FLAG(curr_op), curr);
    else if (GET_FLAG(curr_op) == RELOCATE)
        helpRelocate(DE_FLAG(curr_op), parent, parent_op, curr);
    else if (GET_FLAG(curr_op) == TOMB)
        helpMarked(parent, parent_op, curr);
}

void BST::helpMarked(Node* parent, Operation* parent_op, Node* curr) {
    Node* tmp;
    if (IS_NULL(curr->left)) {
        if (IS_NULL(curr->right)) tmp = SET_NULL(curr);
        else tmp = curr->right;
    }
    else tmp = curr->left;

    bool is_left = curr == parent->left;
    Operation* cas_op = new ChildCASOp(is_left, curr, tmp);
    if (CAS(&parent->op, parent_op, SET_FLAG(cas_op, CHILDCAS))) {
        helpChildCAS(cas_op, parent);
    }
}

void BST::helpChildCAS(Operation* op, Node* dest) {
    ChildCASOp* child_op = (ChildCASOp *) op;
    Node* volatile* addr = child_op->is_left ? &dest->left : &dest->right;
    CAS(address, child_op->expected, child_op->update);
    CAS(&dest->op, SET_FLAG(op, CHILDCAS), SET_FLAG(op, NONE));
}