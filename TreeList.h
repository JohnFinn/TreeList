#pragma once

#include <cassert>
#include <stdexcept>


template <class T>
class TreeList {
public: // just for debugging simplicity

    struct Node{
        T value;
        long diff=0;
        Node *bigger= nullptr, *smaller = nullptr, *parent= nullptr;

        Node(long diff, T value) : diff(diff), value(value) {} // TODO T& and T&&

        // child of this->parent will be node instead of this
        void set_parent_ref(Node* node){
            if (parent){
                if (this == parent->bigger)
                    parent->bigger = node;
                else if (this == parent->smaller)
                    parent->smaller = node;
            }
        }

        // child of this->parent will be node instead of this and node's parent will be this->parent
        void set_parent_son(Node* node){
            set_parent_ref(node);
            if (node)
                node->parent = parent;
        }

        void make_bigger(long differ, T val){
            bigger = new Node(differ, val);
            bigger->parent = this;
        }

        void make_smaller(long differ, T val){
            smaller = new Node(differ, val);
            smaller->parent = this;
        }
        /*
            bigger will be our parent; our bigger child will be bigger smaller child
            this will be SMALLER of bigger child
            I saw that it is called left-rotate, but this naming suits more
        */
        void small_rotate(){
            assert(bigger != nullptr);
            Node *b = bigger, *p = parent;

            set_parent_ref(bigger);
            bigger->parent = parent;

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
            Node *s = smaller, *p = parent;

            set_parent_ref(smaller);
            smaller->parent = parent;

            Node* sb = smaller->bigger;
            smaller->bigger = this; // become big child of our small child
            parent = smaller;

            smaller = sb; // get smaller's bigger child
            if (sb) sb->parent = this;

        }

        // return min node, larger then current
        Node* successor(){
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
    };

    Node* root = nullptr;
public:
    TreeList()= default;
    ~TreeList(){} // TODO remove all Nodes

    // insert value before index
    // if index >= number of items, insert after last
    // TODO pass by &, &&
    void insert(unsigned long index, T value){
        if (root == nullptr){
            root = new Node(0, value);
            return;
        }

        // offset everything after index (inclusive) by one
        Node* current = root;
        unsigned long current_index = current->diff;
        while (true){
            if (index == current_index){
                ++current->diff; // offset with the smaller half. NOTE current index was not given any offset
                ++current_index;
                if (current->smaller){
                    --current->smaller->diff; // unoffset smaller half
                    current = current->smaller;
                    // continue searching to find and insert
                } else {
                    current->make_smaller(-1, value);
                    break;
                }

            } else if (index > current_index){ // don't need to offset anything
                if (not current->bigger) { // found !!!
                    current->make_bigger(1, value); // insert new node
                    break;
                }
                current = current->bigger;
            } else { // index < current_index
                ++current->diff; // offset with the smaller half. current_index wasn't given any offset
                ++current_index;
                if (not current->smaller) {
                    current->make_smaller(-1,value); // unoffset new node (that's why -1 - 1)
                    break;
                }
                --current->smaller->diff; // unoffset smaller half
                // what happened is [x x x] -> [_ x x x] -> [x _ x x]
                current = current->smaller;
            }
            current_index += current->diff;
        }
    }

    // remove value at index
    // offset everything bigger to the left
    // do nothing if no such index
    void remove(unsigned long index){
        if (not root)
            return;

        // looking for node and moving parts of list to the left if we need to
        Node* current = root;
        unsigned long current_index = current->diff;
        while (true){
            if (index == current_index){
                if (current->bigger)
                    --current->bigger->diff; // move right part to the left
                break; // found
            } else if (index > current_index){
                if (not current->bigger)
                    break;
                current = current->bigger;
            } else { // index < current_index
                assert(current->smaller); // in list no missing indices possible
                // move right half to the left, inclusive
                --current->diff; // move right half to the left
                --current_index; // current_index also moved
                ++current->smaller->diff; // move left half right, because we didn't need to move it
                // [x x x x x] -> x [x x x x _] -> [x (xx) x x _]
                // now there are two nodes in the tree with same indices
                current = current->smaller;
            }
            current_index += current->diff;
        }

        assert(current);
        // everything after index must be moved left by 1 at this point

        // removing element
        if (not current->smaller and not current->bigger){
            current->set_parent_ref(nullptr);
            if (current == root)
                root = nullptr;
        }
        else if (not current->smaller and current->bigger){
            if (current == root)
                root = current->bigger;
            if (current->bigger)
                current->bigger->diff += current->diff; // recalculating relative offset
            current->set_parent_son(current->bigger);
        }
        else if (current->smaller and not current->bigger){
            if (current == root)
                root = current->smaller;
            if (current->smaller)
                current->smaller->diff += current->diff; // recalculating relative offset
            current->set_parent_son(current->smaller);
        }
        else { // find next (min, but larger then current) and place here
            Node* successor = current->successor();
            assert(successor != root);
            assert(successor);
            assert(not successor->smaller);
            assert(current->bigger);

            current->value = successor->value; // swap successor and current
            // move successor->bigger subtree up
            if (successor->bigger)
                successor->bigger->diff += successor->diff; // diff are relative to parent, that's why we're changing it
            successor->set_parent_son(successor->bigger);

            current = successor; // trick to free right memory
        }
        delete current; // don't forget to free memory
    }

    // Node at index, nullptr if not exist
    Node* get_node(unsigned long index){
        if (not root) return nullptr;
        Node* current = root;
        unsigned long current_index = current->diff;
        while (true){
            if (current_index == index)
                return current;
            if (index < current_index) {
                assert(current->smaller); // we're implementing a list, not a dictionary
                current = current->smaller;
            } else { // index > current_index
                if (not current->bigger)
                    return nullptr;
                current = current->bigger;
            }
            current_index += current->diff;
        }
    }

    T& operator[](unsigned long index){
        return get_node(index)->value;
    }

    T& at(unsigned long index){
        Node* node = get_node(index);
        if (node)
            return node->value;
        throw std::out_of_range(std::to_string(index) + " is out of range");
    }


    void push_back(T value){
        if (root == nullptr){
            root = new Node(0, value);
            return;
        }

        Node* current = root;
        while (current->bigger)
            current = current->bigger;
        current->bigger = new Node(1, value);
    }

};
