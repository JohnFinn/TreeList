#pragma once

#include <cmath>
#include <algorithm>
#include <cassert>

template <class T>
struct Node{
    T value;
    long diff=0;
    unsigned long height=1; // maximum height
    Node *bigger= nullptr, *smaller = nullptr, *parent= nullptr;

    Node(long diff, const T& value) : diff(diff), value(value) {} // TODO T& and T&&

    bool operator==(const Node& other) const {
        return value == other.value and diff == other.diff and
               height == other.height and bigger == other.bigger and
               smaller == other.smaller and parent == other.parent;
    }

    bool bad_slope() const {
        return std::abs(slope()) > 1;
    }

    bool height_is_correct() const {
        return height == 1 + std::max(sheight(), bheight());
    }

    unsigned long index() const {
        long s = diff;
        for (Node* current = parent; current; current = current->parent)
            s += current->diff;
        assert(s >= 0);
        return s;

    }

    // child of this->parent will be node instead of this
    void set_parent_ref(Node* node){
        if (parent){
            if (this == parent->bigger)
                parent->bigger = node;
            else if (this == parent->smaller)
                parent->smaller = node;
        }
    }

    bool is_smaller() const {
        return parent and parent->smaller == this;
    }

    bool is_bigger() const {
        return parent and parent->bigger == this;
    }

    unsigned long sheight() const {
        if (smaller)
            return smaller->height;
        else
            return 0;
    }

    unsigned long bheight() const {
        if (bigger)
            return bigger->height;
        else
            return 0;
    }

    long slope() const {
        return bheight() - sheight();
    }


    // child of this->parent will be node instead of this and node's parent will be this->parent
    void set_parent_son(Node* node){
        set_parent_ref(node);
        if (node)
            node->parent = parent;
    }

    // creates bigger child
    void make_bigger(long differ, const T& val){
        assert(not bigger);
        bigger = new Node(differ, val);
        bigger->parent = this;
        if (sheight() == 0)
            height = 2;

    }

    // creates smaller child
    void make_smaller(long differ, const T& val){
        assert(not smaller);
        smaller = new Node(differ, val);
        smaller->parent = this;
        if (bheight() == 0)
            height = 2;
    }

    /*
        bigger will be our parent; our bigger child will be bigger smaller child
        this will be SMALLER of bigger child
        I saw that it is called left-rotate, but this naming suits more
    */
    void small_rotate(){
        assert(bigger != nullptr);

        // dealing with heights
        height = 1 + std::max(sheight(), bigger->sheight());
        bigger->height = 1 + std::max(height, bigger->bheight());

        // deal with relative offsets
        long old_diff = diff, old_bdiff = bigger->diff;
        bigger->diff += old_diff; // moving up, including new offset component
        diff -= bigger->diff; // moving down, excluding extra offset component
        if (bigger->smaller) bigger->smaller->diff += old_bdiff;

        Node *b = bigger, *p = parent;

        set_parent_son(bigger);

        Node* bs = bigger->smaller;
        bigger->smaller = this;
        parent = bigger;

        bigger = bs;
        if (bs) bs->parent = this;
    }

    /*
     smaller will be our parent; our smaller child will be smaller bigger child
     this will bi BIGGER of smaller child
     I saw that it is called right-rotate, but this naming suits more
    */
    void big_rotate(){
        assert(smaller != nullptr);

        // dealing with heights
        height = 1 + std::max(bheight(), smaller->bheight());
        smaller->height = 1 + std::max(height, smaller->sheight());

        // deal with relative offsets
        long old_diff = diff, old_sdiff = smaller->diff;
        smaller->diff += old_diff; // moving up, including new offset component
        diff -= smaller->diff; // moving down, excluding extra offset component
        if (smaller->bigger) smaller->bigger->diff += old_sdiff;

        Node *s = smaller, *p = parent;

        set_parent_son(smaller);

        Node* sb = smaller->bigger;
        smaller->bigger = this; // become big child of our small child
        parent = smaller;

        smaller = sb; // get smaller's bigger child
        if (sb) sb->parent = this;

    }

    // return min node, larger then current
    Node* successor() const {
        assert(bigger);
        Node* current = bigger;
        long current_diff = current->diff;
        while (current->smaller) {
            current = current->smaller;
            current_diff += current->diff;
        }
//            assert(current_diff > 0); // result is larger then this, but method TreeList::remove violates this (it ofsets right subtree before finding successor)

        return current;

    }

    // sets proper height, assuming correctness of child heights
    void fix_height(){
        height = 1 + std::max(sheight(), bheight());
    }
};
