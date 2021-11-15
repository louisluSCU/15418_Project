class Operation {

};

class Node {
public:
    int volatile key;
    Operation* volatile op;
    Node* volatile left;
    Node* volatile right;

    Node();
    Node(int k);
};

class ChildCASOp : public Operation {
public:
    bool is_left;
    Node* expected;
    Node* update;

    ChildCASOp(bool is_left, Node* old_n, Node* new_n);
};

class RelocateOp : public Operation {
public:
    int volatile state = 0;
    Node* dest;
    Operation* dest_op;
    int key_to_remove;
    int key_to_put;

    RelocateOp(Node* curr, Operation* curr_op, int curr_key, int new_key);
};

class BST {
    Node root;

public:
    bool contains(int k);
    bool add(int k);
    bool remove(int k);
    BST();

private:
    int find(int k, Node*& parent, Operation*& parent_op, Node*& curr,
         Operation*& curr_op, Node* root);
    void help(Node* parent, Operation* parent_op, Node* curr, Operation* curr_op);
    void helpMarked(Node* parent, Operation* parent_op, Node* curr);
    void helpChildCAS(Operation* op, Node* dest);
    bool helpRelocate(Operation* op, Node* parent, Operation* parentOp, Node* curr);
};
