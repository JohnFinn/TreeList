#pragma once

#include <cmath>
#include <algorithm>
#include <cassert>

template <class T>
struct Node{
    T value;
    long diff=0;
    unsigned long height=1; // maximum height
    Node *right= nullptr, *left = nullptr, *parent= nullptr;

    Node(long diff, T value) : diff(diff), value(std::move(value)) {}

    bool operator==(const Node &other) const noexcept {
        return value == other.value and diff == other.diff and left == other.left and right == other.right and
               height == other.height;
    }

    unsigned long proper_height() const noexcept {
        return 1 + std::max(left_height(), right_height());
    }

    bool bad_slope() const noexcept {
        return std::abs(slope()) > 1;
    }

    bool height_is_correct() const noexcept {
        return height == proper_height();
    }

    unsigned long index() const noexcept {
        long s = diff;
        for (Node* current = parent; current; current = current->parent)
            s += current->diff;
        assert(s >= 0);
        return static_cast<unsigned long>(s);
    }

    // child of this->parent will be node instead of this
    void set_parent_ref(Node* node) noexcept {
        if (parent){
            if (this == parent->right)
                parent->right = node;
            else if (this == parent->left)
                parent->left = node;
        }
    }

    bool is_left() const noexcept {
        return parent and parent->left == this;
    }

    bool is_right() const noexcept {
        return parent and parent->right == this;
    }

    unsigned long left_height() const noexcept {
        return left ? left->height : 0;
    }

    unsigned long right_height() const noexcept {
        return right ? right->height : 0;
    }

    long slope() const noexcept {
        return right_height() - left_height();
    }

    // child of this->parent will be node instead of this and node's parent will be this->parent
    void set_parent_son(Node* node) noexcept {
        set_parent_ref(node);
        if (node)
            node->parent = parent;
    }

    void make_right(Node<T>* node){
        assert(not right);
        right = node;
        right->parent = this;
        if (left_height() == 0)
            height = 2;
    }

    void make_left(Node<T>* node){
        assert(not left);
        left = node;
        left->parent = this;
        if (right_height() == 0)
            height = 2;
    }

    /*
     * X.left_rotate();
          X              Y
         / \            / \
        A   Y   --->   X   C
           / \        / \
          B   C      A   B
    */
    void left_rotate() noexcept {
        assert(right != nullptr);
        // dealing with heights
        height = 1 + std::max(left_height(), right->left_height());
        right->height = 1 + std::max(height, right->right_height());
        // deal with relative offsets
        long old_diff = diff, old_rdiff = right->diff;
        right->diff += old_diff; // moving up, including new offset component
        diff -= right->diff; // moving down, excluding extra offset component
        if (right->left) right->left->diff += old_rdiff;

        set_parent_son(right);

        Node* rl = right->left;
        right->left = this;
        parent = right;

        right = rl;
        if (rl) rl->parent = this;
    }

    /*
     * Y.right_rotate();
          X              Y
         / \            / \
        A   Y   <---   X   C
           / \        / \
          B   C      A   B
    */
    void right_rotate() noexcept {
        assert(left != nullptr);
        // dealing with heights
        height = 1 + std::max(right_height(), left->right_height());
        left->height = 1 + std::max(height, left->left_height());
        // deal with relative offsets
        long old_diff = diff, old_ldiff = left->diff;
        left->diff += old_diff; // moving up, including new offset component
        diff -= left->diff; // moving down, excluding extra offset component
        if (left->right) left->right->diff += old_ldiff;

        set_parent_son(left);

        Node* lr = left->right;
        left->right = this; // become right child of our left child
        parent = left;

        left = lr; // get left's right child
        if (lr) lr->parent = this;
    }

    // return min node, larger then current
    Node* successor() const noexcept {
        assert(right);
        Node* current = right;
        long current_diff = current->diff;
        while (current->left) {
            current = current->left;
            current_diff += current->diff;
        }
        return current;
    }

    void fix_height() noexcept {
        height = proper_height();
    }
};
