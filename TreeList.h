#pragma once

#include "TreeListIterator.cpp"
#include "Node.h"
#include <cassert>
#include <stdexcept>
#include <stack>

// heights of subtrees differ at most by one
template <class T, typename allocator=std::allocator<Node<T>>>
class TreeList {
public: // just for debugging simplicity
    allocator _allocator;
    typedef Node<T> NodeType;
    typedef NodeType* NodePtr;
    NodePtr root = nullptr;
public:
    TreeList()= default;
    ~TreeList(){ clear(); }

    TreeList(TreeList&& other) noexcept : root(other.root) { if (&other != this) other.root = nullptr; }

    TreeList(const TreeList& other){ *this = other; }

    TreeList& operator=(TreeList&& other) noexcept {
        if (this == &other)
            return *this;
        root = other.root;
        other.root = nullptr;
        return *this;
    }

    TreeList& operator=(const TreeList& other){
        // TODO improve performance
        if (this == &other)
            return *this;
        clear();
        for (unsigned long index = 0; true; ++index){
            try{
                insert(index, other.at(index));
            } catch (std::out_of_range& ex){
                break;
            }
        }
        return *this;
    }

    void clear(){
        // TODO improve by iterating through all nodes
        while (root)
            remove(root->diff);
        root = nullptr;
    }

    // insert value before index
    // if index >= number of items, insert after last
    // TODO pass by &, &&
    void insert(unsigned long index, const T& value){
        if (root == nullptr){
            root = this->_allocator.allocate(1);
            new (root) NodeType(0, value);
            return;
        }

        // offset everything after index (inclusive) by one
        NodePtr current = root;
        unsigned long current_index = current->diff;
        while (true){
            if (index == current_index){
                ++current->diff; // offset with the left half. NOTE current index was not given any offset
                ++current_index;
                if (current->left){
                    --current->left->diff; // unoffset left half
                    current = current->left;
                    // continue searching to find and insert
                } else {
                    NodePtr node = this->_allocator.allocate(1);
                    new (node) NodeType(-1, value);
                    current->make_left(node);
                    break;
                }

            } else if (index > current_index){ // don't need to offset anything
                if (not current->right) { // found !!!
                    NodePtr node = this->_allocator.allocate(1);
                    new (node) NodeType(1, value);
                    current->make_right(node); // insert new node
                    break;
                }
                current = current->right;
            } else { // index < current_index
                ++current->diff; // offset with the left half. current_index wasn't given any offset
                ++current_index;
                if (not current->left) {
                    NodePtr node = this->_allocator.allocate(1);
                    new (node) NodeType(-1, value);
                    current->make_left(node);
                    break;
                }
                --current->left->diff; // unoffset left half
                // what happened is [x x x] -> [_ x x x] -> [x _ x x]
                current = current->left;
            }
            current_index += current->diff;
        }

        fix(current);
    }

    // move all elements left that lie after index
    // return Node, that has to be removed, because leftmost Node of moving side will have same index
    // if no Node with index exist, returns nullptr
    NodePtr move_left(unsigned long index){
        if (not root)
            return nullptr;
        // looking for node and moving parts of list to the left if we need to
        NodePtr current = root;
        unsigned long current_index = current->diff;
        while (true){
            if (index == current_index){
                if (current->right)
                    --current->right->diff; // move right part to the left
                return current; // found
            } else if (index > current_index){
                if (not current->right) // index is too big
                    return nullptr;
                current = current->right;
            } else { // index < current_index
                assert(current->left); // in list no missing indices possible
                // move right half to the left, inclusive
                --current->diff; // move right half to the left
                --current_index; // current_index is index of current which is moved
                ++current->left->diff; // move left half right, because we didn't need to move it
                // [x x x x x] -> x [x x x x _] -> [x (xx) x x _]
                // now there are two nodes in the tree with same indices
                current = current->left;
            }
            current_index += current->diff;
        }
    }

    // remove value at index
    // offset everything right to the left
    // do nothing if no such index
    void remove(unsigned long index){

        NodePtr target = move_left(index);
        if (not target)
            return;

        NodePtr parent = target->parent;

        // removing element
        if (not target->left and not target->right){
            target->set_parent_ref(nullptr);
            if (target == root)
                root = nullptr;
        }
        else if (not target->left and target->right){
            if (target == root)
                root = target->right;
            if (target->right)
                target->right->diff += target->diff; // recalculating relative offset
            target->set_parent_son(target->right);
        }
        else if (target->left and not target->right){
            if (target == root)
                root = target->left;
            if (target->left)
                target->left->diff += target->diff; // recalculating relative offset
            target->set_parent_son(target->left);
        }
        else { // find next (min, but larger then target) and place here
            NodePtr successor = target->successor();
            assert(successor != root);
            assert(successor);
            assert(not successor->left);
            assert(target->right);

            target->value = std::move(successor->value); // swap successor and target
            parent = successor->parent; // we will delete successor's node
            // move successor->right subtree up
            if (successor->right)
                successor->right->diff += successor->diff; // diff are relative to parent, that's why we're changing it
            successor->set_parent_son(successor->right);

            target = successor; // trick to free right memory
        }
        this->_allocator.deallocate(target, 1);
        if (not parent) // means root is deleted
            return; // don't need to fix anything if root is deleted (parent is successor's parent)
        else {
            parent->fix_height();
            fix(parent);
        }
    }

    // Node at index, nullptr if not exist
    NodePtr get_node(unsigned long index) const {
        if (not root) return nullptr;
        NodePtr current = root;
        unsigned long current_index = current->diff;
        while (true){
            if (current_index == index)
                return current;
            if (index < current_index) {
                assert(current->left); // we're implementing a list, not a dictionary
                current = current->left;
            } else { // index > current_index
                if (not current->right)
                    return nullptr;
                current = current->right;
            }
            current_index += current->diff;
        }
    }

    T& operator[](unsigned long index) const {
        return get_node(index)->value;
    }

    T& at(unsigned long index) const {
        NodePtr node = get_node(index);
        if (node)
            return node->value;
        throw std::out_of_range(std::to_string(index) + " is out of range");
    }

    void push_back(const T& value){
        if (root == nullptr){
            root = this->_allocator.allocate(1);
            new (root) NodeType(0, value);
            return;
        }

        // find last element
        NodePtr current = root;
        while (current->right)
            current = current->right;

        NodePtr node = this->_allocator.allocate(1);
        new (node) NodeType(1, value);
        current->make_right(node);
        fix(current);

    }

    // accepts parent of inserted/deleted node
    // assumes correct height of node and unfixed height of it's parent
    void fix(NodePtr node){
        long slope = node->slope();
        do {
            assert(slope == 2 or slope == -2 or slope == 1 or slope == -1 or slope == 0);
            if (slope == 2){
                if (node->right->slope() < 0) // == -1, left-heavy,
                    node->right->right_rotate(); // change zig-zag path to zig-zig, not zag-zig
                node->left_rotate();
                node = node->parent; // rotation moved node down
            } else if (slope == -2){
                if (node->left->slope() > 0)
                    node->left->left_rotate();
                node->right_rotate();
                node = node->parent;
            }

            if (not node->parent){ // root is rotated down
                root = node;
                break;
            }
            node = node->parent;
            node->fix_height(); // rotations doesn't change upper heights
            slope = node->slope();
        } while (node->bad_slope() or (node->parent and
                (not node->parent->height_is_correct() or node->parent->bad_slope())));
    }

    TreeListIterator<T> begin(){ // TODO
        return TreeListIterator<T>(this->root);
    }

    TreeListIterator<T> end(){
        return TreeListIterator<T>(nullptr);
    }
};

// output mermaid graph
// each node has index, height, value
template <class T, class stream_t>
stream_t& operator << (stream_t& stream, TreeList<T>& tree){
    typedef typename TreeList<T>::NodePtr Nodeptr;
    if (not tree.root) return stream;

    std::stack<Nodeptr> stack;
    std::stack<unsigned long> indices;
    stack.push(tree.root);

    unsigned long current_index = 0;
    if (tree.root)
        current_index = tree.root->diff;

    indices.push(current_index);

    while (not stack.empty()){
        Nodeptr current = stack.top();
        stack.pop();
        current_index = indices.top();
        indices.pop();
        stream << current << "((" << current_index << ", " << current->height
               << ", " << current->value << "))\n";
        if (current->right) {
          indices.push(current_index + current->right->diff);
          stack.push(current->right);
        }
        if (current->left) {
            indices.push(current_index + current->left->diff);
            stack.push(current->left);
        }

    }

    stack.push(tree.root);

    while (not stack.empty()){
        Nodeptr current = stack.top();
        stack.pop();
        if (current->left){
            stream << current << "-->" << current->left << '\n';
            stack.push(current->left);
        }

        if (current->right) {
            stream << current << "-->" << current->right << '\n';
            stack.push(current->right);
        }
    }

    return stream;
}
