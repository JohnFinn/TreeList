#include "TreeListIterator.h"


template<typename T>
bool TreeListIterator<T>::operator==(const TreeListIterator &other) const noexcept {
    return this->node == other.node;
}

template<typename T>
const T& TreeListIterator<T>::operator*() const noexcept {
    return this->node->value;
}

template<typename T>
TreeListIterator<T>& TreeListIterator<T>::operator++() noexcept {
    if (this->node->right){
        this->node = node->successor();
    } else {
        this->node = node->parent;
    }
    return *this;
}

template<typename T>
TreeListIterator<T>& TreeListIterator<T>::operator=(const TreeListIterator &other) const noexcept {
    this->node = other.node;
    return *this;
}

template<typename T>
bool TreeListIterator<T>::operator!=(const TreeListIterator &other) const noexcept {
    return this->node != other.node;
}

template<typename T>
TreeListIterator<T>::TreeListIterator(const TreeListIterator &other) noexcept : node(other.node) {}
