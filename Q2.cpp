
#include <iostream>
using namespace std;

enum Color {
    RED,
    BLACK
};

struct Node {
    int data;
    Color color;

    Node* left;
    Node* right;
    Node* parent;

    int size;  // Number of nodes in this subtree

    Node(int value = 0) {
        data = value;
        color = RED;
        left = right = parent = nullptr;
        size = 1;
    }
};

class OrderStatisticRBTree {
private:
    Node* root;
    Node* NIL;

    int getSize(Node* node) const {
        return node == NIL ? 0 : node->size;
    }

    Color getColor(Node* node) const {
        return node == NIL ? BLACK : node->color;
    }

    void updateSize(Node* node) {
        if (node != NIL) {
            node->size = 1
                       + getSize(node->left)
                       + getSize(node->right);
        }
    }

    void updateSizeUpwards(Node* node) {
        while (node != NIL) {
            updateSize(node);
            node = node->parent;
        }
    }

    Node* searchNode(int key) const {
        Node* current = root;

        while (current != NIL) {
            if (key == current->data)
                return current;

            if (key < current->data)
                current = current->left;
            else
                current = current->right;
        }

        return NIL;
    }

    Node* minimum(Node* node) const {
        while (node->left != NIL)
            node = node->left;

        return node;
    }

    // Left rotation around x.
    void leftRotate(Node* x) {
        Node* y = x->right;

        x->right = y->left;

        if (y->left != NIL)
            y->left->parent = x;

        y->parent = x->parent;

        if (x->parent == NIL)
            root = y;
        else if (x == x->parent->left)
            x->parent->left = y;
        else
            x->parent->right = y;

        y->left = x;
        x->parent = y;

        // Update the lower node before the new subtree root.
        updateSize(x);
        updateSize(y);
    }

    // Right rotation around y.
    void rightRotate(Node* y) {
        Node* x = y->left;

        y->left = x->right;

        if (x->right != NIL)
            x->right->parent = y;

        x->parent = y->parent;

        if (y->parent == NIL)
            root = x;
        else if (y == y->parent->left)
            y->parent->left = x;
        else
            y->parent->right = x;

        x->right = y;
        y->parent = x;

        updateSize(y);
        updateSize(x);
    }

    // Restore red-black properties after insertion.
    void fixInsert(Node* z) {
        while (z->parent->color == RED) {
            if (z->parent == z->parent->parent->left) {
                Node* uncle = z->parent->parent->right;

                if (getColor(uncle) == RED) {
                    // Case 1: red uncle, recolor.
                    z->parent->color = BLACK;
                    uncle->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {
                        // Case 2: triangle.
                        z = z->parent;
                        leftRotate(z);
                    }

                    // Case 3: line.
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    rightRotate(z->parent->parent);
                }
            } else {
                //Uncle is on the left side. Mirror image of above code.
                Node* uncle = z->parent->parent->left;

                if (getColor(uncle) == RED) {
                    z->parent->color = BLACK;
                    uncle->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        rightRotate(z);
                    }

                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    leftRotate(z->parent->parent);
                }
            }
        }

        root->color = BLACK;
    }

    // Replace subtree rooted at u with subtree rooted at v.
    void transplant(Node* u, Node* v) {
        if (u->parent == NIL)
            root = v;
        else if (u == u->parent->left)
            u->parent->left = v;
        else
            u->parent->right = v;

        // NIL also needs a parent during deletion fix-up.
        v->parent = u->parent;
    }

    // Restore red-black properties after deleting a black node.
    void fixDelete(Node* x) {
        while (x != root && getColor(x) == BLACK) {
            if (x == x->parent->left) {
                Node* sibling = x->parent->right;

                if (sibling == NIL) {
                    x = x->parent;
                    continue;
                }

                // Case 1: sibling is red.
                if (getColor(sibling) == RED) {
                    sibling->color = BLACK;
                    x->parent->color = RED;
                    leftRotate(x->parent);
                    sibling = x->parent->right;
                }

                // Case 2: both sibling children are black.
                if (getColor(sibling->left) == BLACK &&
                    getColor(sibling->right) == BLACK) {
                    sibling->color = RED;
                    x = x->parent;
                } else {
                    // Case 3: far child black, near child red.
                    if (getColor(sibling->right) == BLACK) {
                        sibling->left->color = BLACK;
                        sibling->color = RED;
                        rightRotate(sibling);
                        sibling = x->parent->right;
                    }

                    // Case 4: far child red.
                    sibling->color = x->parent->color;
                    x->parent->color = BLACK;
                    sibling->right->color = BLACK;
                    leftRotate(x->parent);
                    x = root;
                }
            } else {
                // Mirror image: x is the right child.
                Node* sibling = x->parent->left;

                if (sibling == NIL) {
                    x = x->parent;
                    continue;
                }

                // Case 1: sibling is red.
                if (getColor(sibling) == RED) {
                    sibling->color = BLACK;
                    x->parent->color = RED;
                    rightRotate(x->parent);
                    sibling = x->parent->left;
                }

                // Case 2: both sibling children are black.
                if (getColor(sibling->right) == BLACK &&
                    getColor(sibling->left) == BLACK) {
                    sibling->color = RED;
                    x = x->parent;
                } else {
                    // Case 3: far child black, near child red.
                    if (getColor(sibling->left) == BLACK) {
                        sibling->right->color = BLACK;
                        sibling->color = RED;
                        leftRotate(sibling);
                        sibling = x->parent->left;
                    }

                    // Case 4: far child red.
                    sibling->color = x->parent->color;
                    x->parent->color = BLACK;
                    sibling->left->color = BLACK;
                    rightRotate(x->parent);
                    x = root;
                }
            }
        }

        x->color = BLACK;
    }

    void destroy(Node* node) {
        if (node == NIL)
            return;

        destroy(node->left);
        destroy(node->right);
        delete node;
    }

public:
    OrderStatisticRBTree() {
        NIL = new Node();

        NIL->color = BLACK;
        NIL->size = 0;
        NIL->left = NIL;
        NIL->right = NIL;
        NIL->parent = NIL;

        root = NIL;
    }

    ~OrderStatisticRBTree() {
        destroy(root);
        delete NIL;
    }

    Node* search(int key) const {
        Node* result = searchNode(key);
        return result == NIL ? nullptr : result;
    }

    // Insert a distinct key.
    // Returns false if the key already exists.
    bool insert(int key) {
        Node* parent = NIL;
        Node* current = root;

        while (current != NIL) {
            parent = current;

            if (key == current->data)
                return false;

            if (key < current->data)
                current = current->left;
            else
                current = current->right;
        }

        Node* z = new Node(key);
        z->left = NIL;
        z->right = NIL;
        z->parent = parent;
        z->color = RED;
        z->size = 1;

        if (parent == NIL)
            root = z;
        else if (key < parent->data)
            parent->left = z;
        else
            parent->right = z;

        // The new key increases every ancestor's subtree size.
        updateSizeUpwards(parent);

        fixInsert(z);
        return true;
    }

    // Delete a key.
    // Returns false if the key is absent.
    bool erase(int key) {
        Node* z = searchNode(key);

        if (z == NIL)
            return false;

        Node* y = z;
        Node* x;
        Color originalColor = y->color;

        if (z->left == NIL) {
            x = z->right;

            transplant(z, z->right);
            updateSizeUpwards(x->parent);
        } else if (z->right == NIL) {
            x = z->left;

            transplant(z, z->left);
            updateSizeUpwards(x->parent);
        } else {
            // Use the inorder successor.
            y = minimum(z->right);
            originalColor = y->color;
            x = y->right;

            if (y->parent == z) {
                x->parent = y;
            } else {
                Node* oldParent = y->parent;

                transplant(y, y->right);
                updateSizeUpwards(oldParent);

                y->right = z->right;
                y->right->parent = y;
            }

            transplant(z, y);

            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;

            updateSize(y);
            updateSizeUpwards(y->parent);
        }

        delete z;

        if (originalColor == BLACK)
            fixDelete(x);

        // NIL's parent is temporary state used during fixDelete.
        NIL->parent = NIL;

        return true;
    }

    // Return the k-th smallest key (1-based indexing).
    // The assignment guarantees that k is valid.
    int kthSmallest(int k) const {
        Node* current = root;

        while (current != NIL) {
            int leftSize = getSize(current->left);

            if (k == leftSize + 1)
                return current->data;

            if (k <= leftSize) {
                current = current->left;
            } else {
                k -= leftSize + 1;
                current = current->right;
            }
        }

        return 0; // Unreachable for valid k.
    }

    // Rank: number of stored keys strictly smaller than key.
    // Works even if key is absent.
    int rank(int key) const {
        int result = 0;
        Node* current = root;

        while (current != NIL) {
            if (key <= current->data) {
                current = current->left;
            } else {
                result += getSize(current->left) + 1;
                current = current->right;
            }
        }

        return result;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    OrderStatisticRBTree tree;

    // Initial keys: duplicates are ignored without output.
    for (int i = 0; i < n; ++i) {
        int key;
        cin >> key;
        tree.insert(key);
    }

    int q;
    cin >> q;

    while (q--) {
        int type, x;
        cin >> type >> x;

        if (type == 1) {
            if (tree.insert(x)) {
                cout << x << " is inserted\n";
            } else {
                cout << x
                     << " is already present. So no need to insert\n";
            }
        } else if (type == 2) {
            if (tree.erase(x)) {
                cout << x << " is deleted\n";
            } else {
                cout << x
                     << " is not present. So it can not be deleted\n";
            }
        } else if (type == 3) {
            cout << tree.kthSmallest(x) << '\n';
        } else if (type == 4) {
            cout << tree.rank(x) << '\n';
        }
    }

    return 0;
}
