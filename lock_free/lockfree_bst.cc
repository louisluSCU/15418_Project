#include "lockfree_bst.h"
#include <stdint.h>
#include <stdio.h>

#define FLAG_MASK 3UL
#define NULL_MASK 1UL

#define SET_FLAG(op, flag)  ((Operation*) ((uintptr_t)op | flag))
#define GET_FLAG(op)        ((uintptr_t)op & FLAG_MASK)
#define DE_FLAG(op)         ((Operation*) (((uintptr_t)op >> 2) << 2))

#define SET_NULL(node)  ((Node*) ((uintptr_t) node | NULL_MASK))
#define IS_NULL(node)   (((uintptr_t) node & NULL_MASK) == 1UL)

#define CAS(obj, expected, desired) __sync_bool_compare_and_swap(obj, expected, desired)

#define DEBUG 0
#define log(fmt, ...) \
        do { if (DEBUG) {fprintf(stdout, fmt, __VA_ARGS__);} } while (0)

enum op_flag {
    NONE, TOMB, CHILDCAS, RELOCATE
};

enum find_ret {
    FOUND, NOTFOUND_L, NOTFOUND_R, ABORT
};

enum relocate_state {
    ONGOING, SUCCEED, FAILED
};

Node::Node() : key(0) {
    op = NULL;
    left = NULL;
    right = NULL;
    op = SET_FLAG(op, NONE);
    left = SET_NULL(left);
    right = SET_NULL(right);
}

Node::Node(int k) : key(k) {
    op = NULL;
    left = NULL;
    right = NULL;
    op = SET_FLAG(op, NONE);
    left = SET_NULL(left);
    right = SET_NULL(right);
}

ChildCASOp::ChildCASOp(bool is_left, Node* old, Node* new_n) {
    this->is_left = is_left;
    expected = old;
    update = new_n;
}

RelocateOp::RelocateOp(Node* curr, Operation* curr_op, int curr_key, int new_key) {
    state = ONGOING;
    dest = curr;
    dest_op = curr_op;
    key_to_remove = curr_key;
    key_to_put = new_key;
}

BST::BST() {
    root = Node(-1);
}

int BST::find(int k, Node*& parent, Operation*& parent_op, Node*& curr,
         Operation*& curr_op, Node* root) {
    log("Find key %d\n", k);
    int result, current_key;
    Node* next, *last_right;
    Operation* last_right_op;
    bool is_occupied = false;

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
                is_occupied = true;
                break;
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
                break;
            }
        }

        // Search node is occupied, retry
        if (is_occupied) {
            is_occupied = false;
            continue;
        }

        // 1. Make sure no update on last right node that might invalidate current search result
        // 2. Make sure curr has not been changed between reading op and reading its key
        if ((result == FOUND || last_right_op == last_right->op) && curr_op == curr->op) break;
    }
    return result;
}

bool BST::contains(int k) {
    log("Contains key %d\n", k);
    Node* parent, *curr;
    Operation* parent_op, *curr_op;
    return find(k, parent, parent_op, curr, curr_op, &root) == FOUND;
}

bool BST::add(int k) {
    log("Add key %d\n", k);
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
    log("Remove key %d\n", k);
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
            // Restart the search from curr to find the next largest key
            if (find(k, parent, parent_op, replace, replace_op, curr) == ABORT || (curr->op != curr_op))
                continue;
            relocate_op = new RelocateOp(curr, curr_op, k, replace->key);
            // Ensure node to remove cannot be modified by others
            if (CAS(&replace->op, replace_op, SET_FLAG(relocate_op, RELOCATE))) {
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

/* Physically remove node from the tree */
void BST::helpMarked(Node* parent, Operation* parent_op, Node* curr) {
    Node* tmp;
    if (IS_NULL(curr->left)) {
        if (IS_NULL(curr->right)) tmp = SET_NULL(curr);
        else tmp = curr->right;
    }
    else tmp = curr->left;

    bool is_left = (curr == parent->left);
    Operation* cas_op = new ChildCASOp(is_left, curr, tmp);

    // Physical removal by updating parent node
    if (CAS(&parent->op, parent_op, SET_FLAG(cas_op, CHILDCAS))) {
        helpChildCAS(cas_op, parent);
    }
}

/* Physically add new node to the tree */
void BST::helpChildCAS(Operation* op, Node* dest) {
    ChildCASOp* child_op = (ChildCASOp *) op;
    Node* volatile* addr = child_op->is_left ? &dest->left : &dest->right;
    // Update parent node child pointer
    CAS(addr, child_op->expected, child_op->update);
    // Update parent operation field
    CAS(&dest->op, SET_FLAG(op, CHILDCAS), SET_FLAG(op, NONE));
}

/* Physically relocate replace node to removal node */
bool BST::helpRelocate(Operation* op, Node* parent, Operation* parent_op, Node* curr) {
    RelocateOp* relo_op = (RelocateOp *)op;
    bool result;
    int state = relo_op->state;

    // Operate on the remove node
    if (state == ONGOING) {
        CAS(&relo_op->dest->op, relo_op->dest_op, SET_FLAG(relo_op, RELOCATE));
        Operation* seen_op = relo_op->dest->op;
        // Either this thread start removal or other thread start removal
        if ((seen_op == relo_op->dest_op) || (seen_op == SET_FLAG(relo_op, RELOCATE))) {
            CAS(&relo_op->state, ONGOING, SUCCEED);
            state = SUCCEED;
        }
        else {
            CAS(&relo_op->state, ONGOING, FAILED);
            state = relo_op->state;
        }
    }
    if (state == SUCCEED) {
        // Key removal
        CAS(&relo_op->dest->key, relo_op->key_to_remove, relo_op->key_to_put);
        // Op update
        CAS(&relo_op->dest->op, SET_FLAG(relo_op, RELOCATE), SET_FLAG(relo_op, NONE));
    }

    // Clean up the replace node
    result = (state == SUCCEED);
    // In case relocated is called from find
    if (relo_op->dest == curr) return result;

    CAS(&curr->op, SET_FLAG(relo_op, RELOCATE), SET_FLAG(relo_op, result ? TOMB : NONE));
    // If success, mark and remove the replace node
    if (result) {
        if (relo_op->dest == parent) parent_op = SET_FLAG(relo_op, NONE);
        // Since next largest is guaranteed to be leaf node, it's ok to remove it with helpMared
        helpMarked(parent, parent_op, curr);
    }
    return result;
}

void macro_checker() {
    Operation* op = new ChildCASOp(false, NULL, NULL);
    Operation* new_op;
    printf("Origi pointer: %p\n", op);
    new_op = SET_FLAG(op, RELOCATE);
    printf("Pointer flag: %ld\n", GET_FLAG(new_op));
    printf("Flagged pointer: %p\n", new_op);
    printf("Deflagged pointer: %p\n", DE_FLAG(new_op));

    Node* n = new Node();
    Node* nn;
    printf("Origi node: %p\n", n);
    nn = SET_NULL(n);
    printf("Nullify node: %p\n", nn);
    printf("Check null node: %d\n", IS_NULL(nn));
}
