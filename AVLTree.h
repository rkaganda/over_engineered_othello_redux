/*
 * Everything_Trees
 * 
 * Author: Rukundo Kaganda
 * 
 * Note: AVL Tree Implementation
 * 
 * When doing a removal the insert method uses, inorder sucessor
 * the node is replaced with the smallest value in the right subtree
 * 
 * This is different than https://www.cs.usfca.edu/~galles/visualization/AVLtree.html
 * as it uses inorder predeccesor. (largest value in the left tree)
 * 
 * I did some research and it seems both are valid, so I kept my implementation
 * 
 * The 4 transversals implemented are
 * inorder, preorder, postorder, and level order(used in display_tree)
 * 
 * Since the tree is templated there are checks for 
 * the < operator (its assumed if < is supported then > is supported as well)
 * so we can compare T values 
 * and the << operator so we can cout the transversals.
 * 
 * remove, find and insert are implemented using recursion,
 * the single parameter value version of these functions just call the 
 * recursive version with the root parameter
 * 
 */

#ifndef AVLTREE_H
#define AVLTREE_H

#include <stdexcept>
#include <type_traits>
#include <algorithm>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <functional>
#include <map>
using namespace std;

// struct for tree nodes
template <typename T>
struct AVLNode {
    T data;
    AVLNode<T>* left;
    AVLNode<T>* right;
    int height;
    AVLNode(T value) : data(value), left(nullptr), right(nullptr), height(0) {}
};

template <typename T>
class AVLTree {
private:
    AVLNode<T>* root;
    
    // helper function to check if T has a < operator
    // comparator is required for valid AVL insertions
    template <typename U> 
    // U > U is valid
    static auto has_less_operator(int) -> decltype(std::declval<U>() < std::declval<U>(), std::true_type{});

    template <typename U>
    // U > U is is not valid
    static std::false_type has_less_operator(...);

    // store result
    static constexpr bool has_less = decltype(has_less_operator<T>(0))::value;
    
    void display_tree_helper(AVLNode<T>* node, const std::string& prefix, bool is_left) const;

public:
   AVLTree();
   ~AVLTree();
   AVLNode<T>* insert(T value);                         // insert node with value
   AVLNode<T>* insert(AVLNode<T>* parent, T value);     // recursive insert
   bool remove(T value);                                // remove node with value
   bool remove(AVLNode<T>*& node, T value);             // recursive remove
   AVLNode<T>* balance(AVLNode<T>*);                    // balance
   AVLNode<T>* ll_rotation(AVLNode<T>*);
   AVLNode<T>* rr_rotation(AVLNode<T>*);
   AVLNode<T>* lr_rotation(AVLNode<T>*);
   AVLNode<T>* rl_rotation(AVLNode<T>*);
   AVLNode<T>* get_root() const;                        // get the root
   AVLNode<T>* find(T value) const;                     // we can find values
   AVLNode<T>* find(AVLNode<T>*, T value) const;        // recursive find
   int get_height(AVLNode<T>*) const;                   // get the height from the node
   int get_diff(AVLNode<T>*) const;                     // calculate height diff
   void display_tree(AVLNode<T>*) const;
   void display_inorder(AVLNode<T>*) const;
   void display_preorder(AVLNode<T>*) const;
   void display_postorder(AVLNode<T>*) const;
};


// constructor
template <typename T>
AVLTree<T>::AVLTree() : root(nullptr) {}

// destructor
template <typename T>
AVLTree<T>::~AVLTree() {
    // delete tree
    std::function<void(AVLNode<T>*)> delete_subtree = [&](AVLNode<T>* node) {
        if (node) {
            delete_subtree(node->left);
            delete_subtree(node->right);
            delete node;
        }
    };

    delete_subtree(root);
}

// insert node into tree
template <typename T>
AVLNode<T>* AVLTree<T>::insert(AVLNode<T>* parent, T value) {
    // if tree is empty, new node is root
    if (parent==nullptr) {
        root = new AVLNode<T>(value);
        return root;
    }
    
    AVLNode<T>*& child = (value < parent->data) ? parent->left : parent->right;
    child = insert(child, value);
    
    parent->height = 1 + std::max(get_height(parent->left), get_height(parent->right));
    return balance(parent);
}

// insert node into tree
template <typename T>
AVLNode<T>* AVLTree<T>::insert(T value) {
    root = insert(root, value);
    return root;
}


// find node with value
// returns nullptr if value doesn't exist
template <typename T>
AVLNode<T>* AVLTree<T>::find(AVLNode<T>* root, T value) const {
    // if tree is empty, or the root contains the value
    if (root==nullptr || root->data == value) { 
        return root;  // return the root
    }
    
    //use recursion to traverse 
    if (value < root->data) {
        return find(root->left, value); 
    }
    return find(root->right, value);
}


// find node with value
// returns nullptr if value doesn't exist
template <typename T>
AVLNode<T>* AVLTree<T>::find(T value) const {
    root = find(root, value);
    return root;
}


// delete from tree
// returns false if value doesn't exist
template <typename T>
bool AVLTree<T>::remove(AVLNode<T>*& node, T value) {
    if (node == nullptr) {
        return false; // value not found
    }

    if (value < node->data) { // value is left
        bool removed = remove(node->left, value);
        if (removed) {
            node->height = 1 + std::max(get_height(node->left), get_height(node->right));
            node = balance(node); // balance after removal
        }
        return removed;
    } else if (value > node->data) { // value is right
        bool removed = remove(node->right, value);
        if (removed) {
            node->height = 1 + std::max(get_height(node->left), get_height(node->right));
            node = balance(node); // balance after removal
        }
        return removed;
    } else { // value is in this node <> checked
        if (node->left == nullptr && node->right == nullptr) {  // leaf node
            delete node;
            node = nullptr;
        } else if (node->left == nullptr) {  // only right child
            AVLNode<T>* temp = node;
            node = node->right;
            delete temp;
        } else if (node->right == nullptr) {  // only left child
            AVLNode<T>* temp = node;
            node = node->left;
            delete temp;
        } else { // left and right children
            // find smallest (left) node in right tree
            AVLNode<T>* successor = node->right;
            while (successor->left != nullptr) {
                successor = successor->left;
            }

            // replace data with successor
            node->data = successor->data;

            // remove successor
            remove(node->right, successor->data);

            // balance after removal
            node->height = 1 + std::max(get_height(node->left), get_height(node->right));
            node = balance(node);
        }
         
        return true; // value was removed
    }
}


// delete from tree
// returns false if value doesn't exist
template <typename T>
bool AVLTree<T>::remove(T value) {
    return remove(this->root, value);
}

template <typename T>
AVLNode<T>* AVLTree<T>::get_root() const {
    return root;
}

template <typename T>
AVLNode<T>* AVLTree<T>::balance(AVLNode<T>* node) {
    if (get_diff(node) > 1) {
        if (get_diff(node->left) >= 0) {
            node = ll_rotation(node);
        } else {
            node = lr_rotation(node);
        }
    } else if (get_diff(node) < -1) {
        if (get_diff(node->right) <= 0) {
            node = rr_rotation(node);
        } else {
            node = rl_rotation(node);
        }
    }
    return node;
}

template <typename T>
AVLNode<T>* AVLTree<T>::ll_rotation(AVLNode<T>* parent) {
    AVLNode<T>* temp;
    temp = parent->left;
    parent->left = temp->right;
    temp->right = parent;
    
    // update heights
    parent->height = 1 + std::max(get_height(parent->left), get_height(parent->right));
    temp->height = 1 + std::max(get_height(temp->left), get_height(temp->right));
    return temp;
}


template <typename T>
AVLNode<T>* AVLTree<T>::rr_rotation(AVLNode<T>* parent) {
    AVLNode<T>* temp;
    temp = parent->right;
    parent->right = temp->left;
    temp->left = parent;
    
    // update heights
    parent->height = 1 + std::max(get_height(parent->left), get_height(parent->right));
    temp->height = 1 + std::max(get_height(temp->left), get_height(temp->right));
    return temp;
}


template <typename T>
AVLNode<T>* AVLTree<T>::lr_rotation(AVLNode<T>* parent) {
    AVLNode<T>* temp;
    temp = parent->left;
    parent->left = this->rr_rotation(temp);
    return this->ll_rotation(parent);
}


template <typename T>
AVLNode<T>* AVLTree<T>::rl_rotation(AVLNode<T>* parent) {
    AVLNode<T>* temp;
    temp = parent->right;
    parent->right = this->ll_rotation(temp);
    return this->rr_rotation(parent);
}



// recursive height function
template <typename T>
int AVLTree<T>::get_height(AVLNode<T>* node) const {
    return (node == nullptr) ? -1 : node->height;
}


// helper function to get diff
template <typename T>
int AVLTree<T>::get_diff(AVLNode<T>* node) const {
    return this->get_height(node->left) - this->get_height(node->right);
}


// display tree function
// uses helper so our func sig just requires the root node
template <typename T>
void AVLTree<T>::display_tree(AVLNode<T>* node) const {
    if (node == nullptr) {
        std::cout << "Tree is empty." << std::endl;
        return;
    }

    // Call the helper function with initial parameters
    display_tree_helper(node, "", false);
}


// helper function for displaying the tree
// we use this so we can recusivly display branches
template <typename T>
void AVLTree<T>::display_tree_helper(AVLNode<T>* node, const std::string& prefix, bool is_left) const {
    if (node == nullptr) {
        return;
    }

    // prefix for left or right
    std::cout << prefix;

    // add connector
    if (is_left) {
        std::cout << "├── ";
    } else {
        std::cout << "└── ";
    }

    // print the value
    std::cout << node->data << "(" << node->height << ")" << std::endl;

    // recursive call for left and right
    // add prefix based on left or right child
    display_tree_helper(node->left, prefix + (is_left ? "│   " : "    "), true);
    display_tree_helper(node->right, prefix + (is_left ? "│   " : "    "), false);
}



template <typename T>
void AVLTree<T>::display_inorder(AVLNode<T>* node) const {
    // check that T supports << at compile time
    static_assert(
        std::is_same<decltype(std::declval<std::ostream&>() << std::declval<T>()), std::ostream&>::value,
        "Type T does not support output streaming (<<)."
    );

    if (node == nullptr) {
        return;
    }

    // inorder: left, root, right
    display_inorder(node->left); // left recursive
    try {
        std::cout << node->data << " "; // root
    } catch (...) { // assume the runtime is << exception
        throw std::runtime_error("Type T does not support output streaming (<<).");
    }
    display_inorder(node->right); // right recursive
}

template <typename T>
void AVLTree<T>::display_preorder(AVLNode<T>* node) const {
    // check that T supports << at compile time
    static_assert(
        std::is_same<decltype(std::declval<std::ostream&>() << std::declval<T>()), std::ostream&>::value,
        "Type T does not support output streaming (<<)."
    );

    if (node == nullptr) {
        return;
    }

    // preorder: root, left, right
    try {
        std::cout << node->data << node->height << " "; // root
    } catch (...) { // assume the runtime is << exception
        throw std::runtime_error("Type T does not support output streaming (<<).");
    }
    display_preorder(node->left); // left recursive
    display_preorder(node->right); // right recursive
}

template <typename T>
void AVLTree<T>::display_postorder(AVLNode<T>* node) const {
    // Check if T supports the << operator at compile time
    static_assert(
        std::is_same<decltype(std::declval<std::ostream&>() << std::declval<T>()), std::ostream&>::value,
        "Type T does not support output streaming (<<)."
    );

    if (node == nullptr) {
        return;
    }

    // postorder: left, right, root
    display_postorder(node->left); // left recursive
    display_postorder(node->right); // right recursive
    try {
        std::cout << node->data << " "; // root
    } catch (...) {
        throw std::runtime_error("Type T does not support output streaming (<<).");
    }
}


#endif /* VLTTREE_H */

