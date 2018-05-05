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

    Node* root = nullptr;
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
                    current->make_smaller(-1,value);
                    break;
                }
                --current->smaller->diff; // unoffset smaller half
                // what happened is [x x x] -> [_ x x x] -> [x _ x x]
                current = current->smaller;
            }
            current_index += current->diff;
        }

        fix(current);
    }

    // move all elements left that lie after index
    // return Node, that has to be removed, because leftmost Node of moving side will have same index
    // if no Node with index exist, returns nullptr
    Node* move_left(unsigned long index){
        if (not root)
            return nullptr;
        // looking for node and moving parts of list to the left if we need to
        Node* current = root;
        unsigned long current_index = current->diff;
        while (true){
            if (index == current_index){
                if (current->bigger)
                    --current->bigger->diff; // move right part to the left
                return current; // found
            } else if (index > current_index){
                if (not current->bigger) // index is too big
                    return nullptr;
                current = current->bigger;
            } else { // index < current_index
                assert(current->smaller); // in list no missing indices possible
                // move right half to the left, inclusive
                --current->diff; // move right half to the left
                --current_index; // current_index is index of current which is moved
                ++current->smaller->diff; // move left half right, because we didn't need to move it
                // [x x x x x] -> x [x x x x _] -> [x (xx) x x _]
                // now there are two nodes in the tree with same indices
                current = current->smaller;
            }
            current_index += current->diff;
        }
    }

    // remove value at index
    // offset everything bigger to the left
    // do nothing if no such index
    void remove(unsigned long index){

        Node* target = move_left(index);
        if (not target)
            return;

        Node* parent = target->parent;

        // removing element
        if (not target->smaller and not target->bigger){
            target->set_parent_ref(nullptr);
            if (target == root)
                root = nullptr;
        }
        else if (not target->smaller and target->bigger){
            if (target == root)
                root = target->bigger;
            if (target->bigger)
                target->bigger->diff += target->diff; // recalculating relative offset
            target->set_parent_son(target->bigger);
        }
        else if (target->smaller and not target->bigger){
            if (target == root)
                root = target->smaller;
            if (target->smaller)
                target->smaller->diff += target->diff; // recalculating relative offset
            target->set_parent_son(target->smaller);
        }
        else { // find next (min, but larger then target) and place here
            Node* successor = target->successor();
            assert(successor != root);
            assert(successor);
            assert(not successor->smaller);
            assert(target->bigger);

            target->value = std::move(successor->value); // swap successor and target
            parent = successor->parent; // we will delete successor's node
            // move successor->bigger subtree up
            if (successor->bigger)
                successor->bigger->diff += successor->diff; // diff are relative to parent, that's why we're changing it
            successor->set_parent_son(successor->bigger);

            target = successor; // trick to free right memory
        }
        delete target; // don't forget to free memory
        if (not parent) // means root is deleted
            return; // don't need to fix anything if root is deleted (parent is successor's parent)
        else {
            parent->fix_height();
            fix(parent);
        }
    }

    // Node at index, nullptr if not exist
    Node* get_node(unsigned long index) const {
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


    T& operator[](unsigned long index) const {
        return get_node(index)->value;
    }


    T& at(unsigned long index) const {
        Node* node = get_node(index);
        if (node)
            return node->value;
        throw std::out_of_range(std::to_string(index) + " is out of range");
    }


    void set(unsigned long index, const T& value){ at(index) = value; }


    void set(unsigned long index, T&& value){ at(index) = std::move(value); }


    void push_back(const T& value){
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
        long slope = node->slope();
        do {
            assert(slope == 2 or slope == -2 or slope == 1 or slope == -1 or slope == 0);
            if (slope == 2){
                if (node->bigger->slope() < 0) // == -1, left-heavy,
                    node->bigger->big_rotate(); // change zig-zag path to zig-zig, not zag-zig
                node->small_rotate();
                node = node->parent; // rotation moved node down
            } else if (slope == -2){
                if (node->smaller->slope() > 0)
                    node->smaller->small_rotate();
                node->big_rotate();
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

};

// output mermaid graph
// each node has index, height, value
template <class T, class stream_t>
stream_t& operator << (stream_t& stream, TreeList<T>& tree){
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
        stream << current << "((" << current_index << ", " << current->height
               << ", " << current->value << "))\n";
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
