#pragma once

#include <cassert>
#include <stdexcept>
#include <stack>

// heights of subtrees differ at most by one
template <class T>
class TreeList {
public: // just for debugging simplicity


    struct Node{
        T value;
        long diff=0;
        unsigned long height=1; // maximum height
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

        bool is_smaller(){
            return parent and parent->smaller == this;
        }

        bool is_bigger(){
            return parent and parent->bigger == this;
        }

        unsigned long sheight(){
            if (smaller)
                return smaller->height;
            else
                return 0;
        }

        unsigned long bheight(){
            if (bigger)
                return bigger->height;
            else
                return 0;
        }

        long slope(){
            return bheight() - sheight();
        }


        // child of this->parent will be node instead of this and node's parent will be this->parent
        void set_parent_son(Node* node){
            set_parent_ref(node);
            if (node)
                node->parent = parent;
        }

        // creates bigger child
        void make_bigger(long differ, T val){
            assert(not bigger);
            bigger = new Node(differ, val);
            bigger->parent = this;
            if (sheight() == 0)
                height = 2;

        }

        // creates smaller child
        void make_smaller(long differ, T val){
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

        // sets proper height, assuming correctness of child heights
        void fix_height(){
            height = 1 + std::max(sheight(), bheight());
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
//                if (current->bigger_count() > current->smaller_count())

                if (not current->bigger) { // found !!!
                    current->make_bigger(1, value); // insert new node
                    break;
                }
                current = current->bigger;
            } else { // index < current_index
                ++current->diff; // offset with the smaller half. current_index wasn't given any offset
                ++current_index;
                if (not current->smaller) {
                    current->make_smaller(-1,value);
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

        // find last element
        Node* current = root;
        while (current->bigger)
            current = current->bigger;

        current->make_bigger(1, value);
        fix(current);

    }

    // accepts parent of inserted/deleted node
    // assumes correct height of node and unfixed height of it's parent
    void fix(Node* node){
        assert(node->bigger != nullptr or node->smaller != nullptr);
        if (not node->parent)
            return;

        node = node->parent;
        long prev_height = node->height;
        node->fix_height();
        long height_change = node->height - prev_height;

        long slope = node->slope();

        while (std::abs(slope + height_change) > 1) {
            assert(slope == 2 or slope == -2 or slope == 1 or slope == -1);
            if (slope == 2){
                if (node->bigger->slope() < 0) // == -1, left-heavy
                    node->bigger->big_rotate();
                node->small_rotate();
                node = node->parent;
            } else if (slope == -2){
                if (node->smaller->slope() < 0)
                    node->smaller->small_rotate();
                node->big_rotate();
                node = node->parent;
            }

            if (not node->parent){
                root = node;
                break;
            }
            node = node->parent; // one is due to rotation, another is because we're going up
            prev_height = node->height;
            node->fix_height();
            height_change = node->height - prev_height;
            slope = node->slope();
        }
    }

};

// output mermaid graph
// each node has index, height, diff
template <class T, class stream_t>
stream_t& operator << (stream_t& stream, TreeList<T> tree){
    typedef typename TreeList<T>::Node* Nodeptr;
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
        stream << current << "((" << current_index << ", " << current->height << ", " << current->diff << "))\n";
        if (current->bigger) {
            indices.push(current_index + current->bigger->diff);
            stack.push(current->bigger);
        }
        if (current->smaller) {
            indices.push(current_index + current->smaller->diff);
            stack.push(current->smaller);
        }

    }

    stack.push(tree.root);

    while (not stack.empty()){
        Nodeptr current = stack.top();
        stack.pop();
        if (current->smaller){
            stream << current << "-->" << current->smaller << '\n';
            stack.push(current->smaller);
        }

        if (current->bigger) {
            stream << current << "-->" << current->bigger << '\n';
            stack.push(current->bigger);
        }
    }

    return stream;
}
