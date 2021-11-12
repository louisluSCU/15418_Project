class Node {
public:
    int volatile key;
    Operation* volatile op;
    Node* volatile left;
    Node* volatile right;
};

class Operation {

};

class ChildCASOp : Operation {
    bool isLeft;
    Node* expected;
    Node* update;
};

class RelocateOp : Operation {
    int volatile state = 0;
    Node* dest;
    Operation* destOp;
    int key_to_remove;
    int key_to_put;
};

class BST {
    Node root;

public:
    bool contains(int k);
    bool add(int k);
    bool remove(int k);

private:
    int find(int k, Node*& parent, Operation*& parent_op, Node*& curr,
         Operation*& curr_op, Node* root);
    void help(Node* parent, Operation* parent_op, Node* curr, Operation* curr_op);
    void helpChildCAS(Operation* op, Node* dest);
    bool helpRelocate(Operation* op, Node* parent, Operation* parentOp, Node* curr);
};
