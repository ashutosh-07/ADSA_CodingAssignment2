#include <iostream>
using namespace std;
#include <queue>

int MAX_KEYS;
int MIN_KEYS;

struct BTreeNode {
int *key; // allocate MAX entries
int count;
struct BTreeNode **children; // allocate MAX + 1 entries
};

BTreeNode* createNode() {
    BTreeNode* node = new BTreeNode;
    node->key = new int[MAX_KEYS];
    node->children = new BTreeNode*[MAX_KEYS + 1];
    node->count = 0;

    for (int i = 0; i <= MAX_KEYS; ++i)
    {
        node->children[i] = nullptr;
    }
        
    return node;
}

bool isLeaf(BTreeNode* node) {
    return node->children[0] == nullptr;
}

// ---SEARCH---
bool search(BTreeNode* root, int target) {
    BTreeNode* current = root;

    while (current != nullptr) {
        int i = 0;

        while (i < current->count && target > current->key[i])
        {
            ++i;
        }
            
        if (i < current->count && current->key[i] == target)
        {
            return true;
        }
           

        current = current->children[i];
    }

    return false;
}


// ---INSERTION--- 

// Takes parent as input and its index child is full we create a new child node
// move the full childs keys and 
void splitChild(BTreeNode* parent, int index) {
    BTreeNode* fullLeft = parent->children[index]; // left child of parent
    BTreeNode* right = createNode();

    // fullLeft is when #key = MAX_KEYS (MAX_KEYS = 2 x MIN_KEYS +1) 
    // so mid would be MIN_KEYS as zero based indexing
    int middleKey = fullLeft->key[MIN_KEYS];

    //Moving keys from fullLeft to right new sibiling
    right->count = MIN_KEYS;

    for (int j = 0; j < MIN_KEYS; ++j)
        right->key[j] = fullLeft->key[MIN_KEYS + 1 + j];

    //Need to move the child pointer as well
    if (!isLeaf(fullLeft)) {
        for (int j = 0; j <= MIN_KEYS; ++j) {
            right->children[j] =
                fullLeft->children[MIN_KEYS + 1 + j];

            fullLeft->children[MIN_KEYS + 1 + j] = nullptr;
        }
    }

    fullLeft->count = MIN_KEYS;

    // Make room for the promoted middle key in parent 

    for (int j = parent->count; j >= index + 1; --j)
        parent->children[j + 1] = parent->children[j];

    parent->children[index + 1] = right;

    for (int j = parent->count - 1; j >= index; --j)
        parent->key[j + 1] = parent->key[j];

    parent->key[index] = middleKey;
    ++(parent->count);
}

//Insertion always happen at leaf node so we recursively traverse to the leaf
//meanwhile if we find any full child in between we split it
void insertNonFull(BTreeNode* node, int value) {
    int i = node->count - 1;

    if (isLeaf(node)) {
        while (i >= 0 && node->key[i] > value) {
            node->key[i + 1] = node->key[i];
            --i;
        }

        node->key[i + 1] = value;
        ++node->count;
        return;
    }

    while (i >= 0 && node->key[i] > value)
        --i;

    ++i;

    // Split a full child before descending into it.
    if (node->children[i]->count == MAX_KEYS) {
        splitChild(node, i);

        if (value > node->key[i])
            ++i;
    }

    insertNonFull(node->children[i], value);
}

// Returns false for a duplicate key else true.
bool insert(BTreeNode*& root, int value) {
    if (search(root, value))
        return false;

    if (root == nullptr) {
        root = createNode();
        root->key[0] = value;
        root->count = 1;
        return true;
    }

    // Split the root before insertion if it is full.
    if (root->count == MAX_KEYS) {
        BTreeNode* newRoot = createNode();
        newRoot->children[0] = root;

        //Splitchild would split the root in 2 and move the middle child up
        splitChild(newRoot, 0);
        root = newRoot;
    }

    insertNonFull(root, value);
    return true;
}

//--- LEVELORDER---

void displayLevelOrder(BTreeNode* root)
{
    if (root == nullptr)
    {
        cout << "EMPTY" << endl;
        cout << "END" << endl;
        return;
    }

    queue<vector<BTreeNode*>> q;

    vector<BTreeNode*> v;
    v.push_back(root);
    q.push(v);

    while (!q.empty())
    {
        vector<BTreeNode*> v = q.front();
        vector<BTreeNode*> res;
        q.pop();

        for (int i = 0; i < v.size(); i++)
        {
            BTreeNode* temp = v[i];

            // Separate BTreeNodes on the same level with spaces.
            if (i > 0)
                cout << " ";

            // Print keys inside this B-Tree BTreeNode, separated by commas.
            for (int j = 0; j < temp->count; j++)
            {
                if (j > 0)
                    cout << ",";

                cout << temp->key[j];
            }

            // Collect all non-null children from left to right.
            for (int j = 0; j <= temp->count; j++)
            {
                if (temp->children[j] != nullptr)
                    res.push_back(temp->children[j]);
            }
        }

        if (!res.empty())
            q.push(res);

        cout << endl;
    }

    cout << "END" << endl;
}

// ---------------- DELETION HELPERS ----------------

void borrowFromPrevious(BTreeNode* parent, int index) {
    BTreeNode* child = parent->children[index];
    BTreeNode* sibling = parent->children[index - 1];

    // Shift child keys right.
    for (int j = child->count - 1; j >= 0; --j)
        child->key[j + 1] = child->key[j];

    // Shift child pointers right for an internal node.
    if (!isLeaf(child)) {
        for (int j = child->count; j >= 0; --j)
            child->children[j + 1] = child->children[j];

        child->children[0] =
            sibling->children[sibling->count];

        sibling->children[sibling->count] = nullptr;
    }

    // Move the parent separator down and sibling key up.
    child->key[0] = parent->key[index - 1];

    parent->key[index - 1] =
        sibling->key[sibling->count - 1];

    ++child->count;
    --sibling->count;
}

void borrowFromNext(BTreeNode* parent, int index) {
    BTreeNode* child = parent->children[index];
    BTreeNode* sibling = parent->children[index + 1];

    child->key[child->count] = parent->key[index];

    if (!isLeaf(child))
        child->children[child->count + 1] =
            sibling->children[0];

    parent->key[index] = sibling->key[0];

    // Shift sibling keys and pointers left.
    for (int j = 1; j < sibling->count; ++j)
        sibling->key[j - 1] = sibling->key[j];

    if (!isLeaf(sibling)) {
        for (int j = 1; j <= sibling->count; ++j)
            sibling->children[j - 1] =
                sibling->children[j];

        sibling->children[sibling->count] = nullptr;
    }

    ++child->count;
    --sibling->count;
}

// Merge left and right child to left node and delete the right node entirely
void mergeChildren(BTreeNode* parent, int index) {
    BTreeNode* left = parent->children[index];
    BTreeNode* right = parent->children[index + 1];

    int leftCount = left->count;

    left->key[leftCount] = parent->key[index];

    for (int j = 0; j < right->count; ++j)
        left->key[leftCount + 1 + j] = right->key[j];

    if (!isLeaf(left)) {
        for (int j = 0; j <= right->count; ++j) {
            left->children[leftCount + 1 + j] =
                right->children[j];

            right->children[j] = nullptr;
        }
    }

    left->count = leftCount + 1 + right->count;

    // Remove the separator from the parent.
    for (int j = index + 1; j < parent->count; ++j)
        parent->key[j - 1] = parent->key[j];

    // Remove the right child's pointer.
    for (int j = index + 2; j <= parent->count; ++j)
        parent->children[j - 1] = parent->children[j];

    parent->children[parent->count] = nullptr;
    --parent->count;

    delete[] right->key;
    delete[] right->children;
    delete right;
}

int getPredecessor(BTreeNode* node) {
    while (!isLeaf(node))
        node = node->children[node->count];

    return node->key[node->count - 1];
}

int getSuccessor(BTreeNode* node) {
    while (!isLeaf(node))
        node = node->children[0];

    return node->key[0];
}

// ---------------- DELETION ----------------

void deleteFromNode(BTreeNode* node, int value) {
    int index = 0;

    while (index < node->count &&
           node->key[index] < value)
        ++index;

    // Case 1: The key is present in this node.
    if (index < node->count &&
        node->key[index] == value) {

        // Case 1a: Key is in a leaf node
        if (isLeaf(node)) {
            for (int j = index + 1; j < node->count; ++j)
            {
                node->key[j - 1] = node->key[j];
            }
            --(node->count);
            return;
        }

        BTreeNode* left = node->children[index];
        BTreeNode* right = node->children[index + 1];

        // Case 1b: Use predecessor if left can lend
        if (left->count > MIN_KEYS) {
            int predecessor = getPredecessor(left);
            node->key[index] = predecessor;
            deleteFromNode(left, predecessor);
        }
        // Use successor if right can lend
        else if (right->count > MIN_KEYS) {
            int successor = getSuccessor(right);
            node->key[index] = successor;
            deleteFromNode(right, successor);
        }
        // Neither can lend, merge
        else {
            mergeChildren(node, index);
            deleteFromNode(node->children[index], value);
        }

        return;
    }

    // Case 2: The key is not in this node.
    if (node->children[index]->count == MIN_KEYS) {

        // Borrow from the left sibling
        if (index > 0 &&
            node->children[index - 1]->count > MIN_KEYS) {

            borrowFromPrevious(node, index);
        }
        // Borrow from the right sibling
        else if (index < node->count &&
                 node->children[index + 1]->count > MIN_KEYS) {

            borrowFromNext(node, index);
        }
        // Merge with right if present
        else if (index < node->count) {
            mergeChildren(node, index);
        }
        // Otherwise merge with the left sibling.
        else {
            mergeChildren(node, index - 1);
            --index;
        }
    }

    deleteFromNode(node->children[index], value);
}

// Returns false for an absent key.
bool deleteKey(BTreeNode*& root, int value) {
    if (!search(root, value))
        return false;

    deleteFromNode(root, value);

    // Replace an empty root with its only child or 
    // make the tree empty if the root was a leaf.
    if (root != nullptr && root->count == 0) {
        BTreeNode* oldRoot = root;

        if (isLeaf(oldRoot)) {
            root = nullptr;
        } else {
            root = oldRoot->children[0];
            oldRoot->children[0] = nullptr;
        }

        delete[] oldRoot->key;
        delete[] oldRoot->children;
        delete oldRoot;
    }

    return true;
}


int main()
{
    BTreeNode *root;

    // Build Initial_BST
    int N;
    cin >> N>> MAX_KEYS>> MIN_KEYS;
    for (int i = 0; i < N; i++)
    {
        int key;
        cin >> key;
        insert(root, key);
    }
    int q;
    cin>>q;

    for(int i=0;i<q;i++)
    {
        int type;
        cin >> type;

        if (type == 4) {
            displayLevelOrder(root);
            continue;
        }
        int value;
        cin >> value;

        if (type == 1) {
            if (search(root, value))
                cout << value << " is present\n";
            else
                cout << value << " is not present\n";
        }
        else if (type == 2) {
            if (insert(root, value))
                cout << value << " is inserted\n";
            else
                cout << value << " is already present\n";
        }
        else if (type == 3) {
            if (deleteKey(root, value))
                cout << value << " is deleted\n";
            else
                cout << value
                     << " is not present. So it can not be deleted\n";
        }
    }

}