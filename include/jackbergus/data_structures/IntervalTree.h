#ifndef __INTERVAL_TREE_H
#define __INTERVAL_TREE_H

#include <vector>
#include <algorithm>
#include <iostream>
#include <memory>
#include <cassert>
#include <limits>
#include <ostream>
#include <functional>

#ifdef USE_INTERVAL_TREE_NAMESPACE
namespace interval_tree {
#endif
// Structure to represent an interval

template<typename T = double>
struct Interval {
    T low, high;

    Interval(const T& low, const T& high) : low(std::min(low, high)),
    high(std::max(low, high)) {}

    friend std::ostream & operator<<(std::ostream &os, const Interval &obj) {
        return os
               << "[" << obj.low
               << ", " << obj.high<< "]";
    }
};

// Structure to represent a node in Interval Search Tree
template<typename T = double>
struct Node {
    Interval<T> *i;
    T max;
    Node *left, *right;
};

// A utility function to create a new Interval Search Tree Node
template<typename T = double>
Node<T> * newNode(const Interval<T>& i) {
    Node<T> *temp = new Node<T>();
    temp->i = new Interval(i);
    temp->max = i.high;
    temp->left = temp->right = nullptr;
    return temp;
};


    template<typename T = double>
Node<T> *insertNode(Node<T> **root, const Interval<T>& i) {
        if ((!root))
            return nullptr;
        Node<T> * insertedNode = nullptr;
        if (root) {
            // Base case: Tree is empty, new node becomes root
            if (*root == nullptr) {
                insertedNode = newNode(i);
                *root = insertedNode;
            } else {
                // Get low value of interval at root
                int l = (*root)->i->low;

                // If root's low value is smaller,
                // then new interval goes to left subtree
                if (i.low < l)
                    insertedNode = insertNode(&(*root)->left, i);

                // Else, new node goes to right subtree.
                else
                    insertedNode = insertNode(&(*root)->right, i);

                // Update the max value of this ancestor if needed
                if ((*root)->max < i.high)
                    (*root)->max = i.high;
            }

        } else {
            // noop
        }
        return insertedNode;
    }

// A utility function to insert a new Interval Search Tree Node
// This is similar to BST Insert.  Here the low value of interval
// is used tomaintain BST property
    template<typename T = double>
Node<T> *insert(Node<T> *root, const Interval<T>& i) {

    // Base case: Tree is empty, new node becomes root
    if (root == nullptr)
        return newNode(i);

    // Get low value of interval at root
    int l = root->i->low;

    // If root's low value is smaller,
    // then new interval goes to left subtree
    if (i.low < l)
        root->left = insert(root->left, i);

    // Else, new node goes to right subtree.
    else
        root->right = insert(root->right, i);

    // Update the max value of this ancestor if needed
    if (root->max < i.high)
        root->max = i.high;

    return root;
}

// A utility function to check if given two intervals overlap
    template<typename T = double>
    bool isOverlapping(const Interval<T>& i1, const Interval<T>& i2) {
    if (i1.low <= i2.high && i2.low <= i1.high)
        return true;
    return false;
}

// The main function that searches a given
// interval i in a given Interval Tree.
    template<typename T = double>
    Interval<T> *overlapSearch(Node<T> *root, const Interval<T>& i) {

    // Base Case, tree is empty
    if (root == nullptr) return nullptr;

    // If given interval overlaps with root
    if (isOverlapping(*(root->i), i))
        return root->i;

    // If left child of root is present and max of left child is
    // greater than or equal to given interval, then i may
    // overlap with an interval is left subtree
    if (root->left != nullptr && root->left->max >= i.low)
        return overlapSearch(root->left, i);

    // Else interval can only overlap with right subtree
    return overlapSearch(root->right, i);
}

    template<typename T = double>
    void inorder(Node<T> *root, const std::function<void(Node<T>*)>& callback) {
        if (root == nullptr)
            return;
        if (root->left)
            inorder(root->left, callback);

        if (root->right)
            inorder(root->right, callback);
        callback(root);
    }

    template<typename T = double>
struct IntervalTree {
    IntervalTree() {

    }
    IntervalTree(const IntervalTree&) = delete;
    IntervalTree(IntervalTree&&) = delete;
    IntervalTree& operator=(const IntervalTree&) = delete;
    IntervalTree& operator=(IntervalTree&&) = delete;

    Node<T>* insertInterval(Interval<T> i) {
        return insertNode(&root, i);
    }

    Interval<T> *lookup(Interval<T> i) {
        return overlapSearch(root, i);
    }

    void clear() {
        std::function<void(Node<T>*)> clear = [](Node<T>* ptr) { delete ptr; };
        inorder(root, clear);
        root = nullptr;
    }

    ~IntervalTree() {
        clear();
    }
private:
    Node<T>* root = nullptr;

};

// void inorder(Node *root) {
//     if (root == nullptr) return;
//     inorder(root->left);
//     cout << "[" << root->i->low << ", " << root->i->high << "]"
//          << " max = " << root->max << endl;
//     inorder(root->right);
// }
#ifdef USE_INTERVAL_TREE_NAMESPACE
}
#endif

#endif