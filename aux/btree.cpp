#include <iostream>
#include <string>
#include <fstream>

using namespace std;

const int MAX_KEYS = 5; 

struct KeyOffset {
    string key;
    streampos offset;
};

class BTreeNode {
public:
    KeyOffset keys[MAX_KEYS - 1];    
    BTreeNode* children[MAX_KEYS]; 
    int n; // Número atual de chaves
    bool leaf; 

    BTreeNode(bool isLeaf = true) : n(0), leaf(isLeaf) {
        for (int i = 0; i < MAX_KEYS; i++)
            children[i] = nullptr;
    }
};

class BTree {
private:
    BTreeNode* root;

    int getMinKeys() {
        return (MAX_KEYS - 1) / 2;
    }

    void split(BTreeNode* x, int i) {
        BTreeNode* y = x->children[i];
        BTreeNode* z = new BTreeNode(y->leaf);
        
        int midIdx = getMinKeys();
        z->n = (MAX_KEYS - 1) - midIdx - 1;

        for (int j = 0; j < z->n; j++) {
            z->keys[j] = y->keys[j + midIdx + 1];
        }

        if (!y->leaf) {
            for (int j = 0; j <= z->n; j++)
                z->children[j] = y->children[j + midIdx + 1];
        }

        y->n = midIdx;

        for (int j = x->n; j >= i + 1; j--)
            x->children[j + 1] = x->children[j];

        x->children[i + 1] = z;

        for (int j = x->n - 1; j >= i; j--)
            x->keys[j + 1] = x->keys[j];

        x->keys[i] = y->keys[midIdx];
        x->n = x->n + 1;
    }

    void insertNonFull(BTreeNode* x, string k, streampos offset) {
        int i = x->n - 1;

        if (x->leaf) {
            while (i >= 0 && k < x->keys[i].key) {
                x->keys[i + 1] = x->keys[i];
                i--;
            }
            x->keys[i + 1] = {k, offset};
            x->n = x->n + 1;
        } else {
            while (i >= 0 && k < x->keys[i].key)
                i--;

            i++;
            if (x->children[i]->n == MAX_KEYS - 1) {
                split(x, i);
                if (k > x->keys[i].key)
                    i++;
            }
            insertNonFull(x->children[i], k, offset); 
        }
    }

    streampos search(BTreeNode* x, string k) {
        int i = 0;
        while (i < x->n && k > x->keys[i].key)
            i++;

        if (i < x->n && k == x->keys[i].key)
            return x->keys[i].offset;

        if (x->leaf)
            return streampos(-1); 

        return search(x->children[i], k);
    }

    KeyOffset getPredecessor(BTreeNode* node, int idx) {
        BTreeNode* current = node->children[idx];
        while (!current->leaf)
            current = current->children[current->n];
        return current->keys[current->n - 1];
    }

    KeyOffset getSuccessor(BTreeNode* node, int idx) {
        BTreeNode* current = node->children[idx + 1];
        while (!current->leaf)
            current = current->children[0];
        return current->keys[0];
    }

    void fill(BTreeNode* node, int idx) {
        int minKeys = getMinKeys();
        if (idx != 0 && node->children[idx - 1]->n > minKeys)
            borrowFromPrev(node, idx);
        else if (idx != node->n && node->children[idx + 1]->n > minKeys)
            borrowFromNext(node, idx);
        else {
            if (idx != node->n)
                merge(node, idx);
            else
                merge(node, idx - 1);
        }
    }

    void borrowFromPrev(BTreeNode* node, int idx) {
        BTreeNode* child = node->children[idx];
        BTreeNode* sibling = node->children[idx - 1];

        for (int i = child->n - 1; i >= 0; i--)
            child->keys[i + 1] = child->keys[i];

        if (!child->leaf) {
            for (int i = child->n; i >= 0; i--)
                child->children[i + 1] = child->children[i];
        }

        child->keys[0] = node->keys[idx - 1];

        if (!child->leaf)
            child->children[0] = sibling->children[sibling->n];

        node->keys[idx - 1] = sibling->keys[sibling->n - 1];

        child->n += 1;
        sibling->n -= 1;
    }

    void borrowFromNext(BTreeNode* node, int idx) {
        BTreeNode* child = node->children[idx];
        BTreeNode* sibling = node->children[idx + 1];

        child->keys[child->n] = node->keys[idx];

        if (!child->leaf)
            child->children[child->n + 1] = sibling->children[0];

        node->keys[idx] = sibling->keys[0];

        for (int i = 1; i < sibling->n; ++i)
            sibling->keys[i - 1] = sibling->keys[i];

        if (!sibling->leaf) {
            for (int i = 1; i <= sibling->n; ++i)
                sibling->children[i - 1] = sibling->children[i];
        }

        child->n += 1;
        sibling->n -= 1;
    }

    void merge(BTreeNode* node, int idx) {
        BTreeNode* child = node->children[idx];
        BTreeNode* sibling = node->children[idx + 1];

        child->keys[child->n] = node->keys[idx];

        for (int i = 0; i < sibling->n; ++i)
            child->keys[i + child->n + 1] = sibling->keys[i];

        if (!child->leaf) {
            for (int i = 0; i <= sibling->n; ++i)
                child->children[i + child->n + 1] = sibling->children[i];
        }

        child->n += sibling->n + 1;

        for (int i = idx + 1; i < node->n; ++i)
            node->keys[i - 1] = node->keys[i];

        for (int i = idx + 2; i <= node->n; ++i)
            node->children[i - 1] = node->children[i];

        node->n--;
        delete sibling;
    }

    void removeFromNonLeaf(BTreeNode* node, int idx) {
        string k = node->keys[idx].key;
        int minKeys = getMinKeys();

        if (node->children[idx]->n > minKeys) {
            KeyOffset pred = getPredecessor(node, idx);
            node->keys[idx] = pred;
            remove(node->children[idx], pred.key);
        } else if (node->children[idx + 1]->n > minKeys) {
            KeyOffset succ = getSuccessor(node, idx);
            node->keys[idx] = succ;
            remove(node->children[idx + 1], succ.key);
        } else {
            merge(node, idx);
            remove(node->children[idx], k);
        }
    }

    void removeFromLeaf(BTreeNode* node, int idx) {
        for (int i = idx + 1; i < node->n; ++i)
            node->keys[i - 1] = node->keys[i];
        node->n--;
    }

    void remove(BTreeNode* node, string k) {
        int idx = 0;
        while (idx < node->n && node->keys[idx].key < k)
            ++idx;

        if (idx < node->n && node->keys[idx].key == k) {
            if (node->leaf)
                removeFromLeaf(node, idx);
            else
                removeFromNonLeaf(node, idx);
        } else {
            if (node->leaf) {
                cout << "A hash" << k << " não está na árvore\n";
                return;
            }

            bool flag = ((idx == node->n) ? true : false);

            if (node->children[idx]->n <= getMinKeys())
                fill(node, idx);

            if (flag && idx > node->n)
                remove(node->children[idx - 1], k);
            else
                remove(node->children[idx], k);
        }
    }

public:
    BTree() { root = new BTreeNode(true); }

    void insert(string k, streampos offset) { 
        if (root->n == MAX_KEYS - 1) {
            BTreeNode* s = new BTreeNode(false);
            s->children[0] = root;
            root = s;
            split(s, 0);
            insertNonFull(s, k, offset);
        } else
            insertNonFull(root, k, offset);
    }

    streampos search(string k) {
        return (root == nullptr) ? streampos(-1) : search(root, k);
    }

    void remove(string k) {
        if (!root) {
            cout << "A árvore esta vazia\n";
            return;
        }

        remove(root, k);

        if (root->n == 0) {
            BTreeNode* tmp = root;
            if (root->leaf)
                root = nullptr;
            else
                root = root->children[0];

            delete tmp;
        }
    }
};
