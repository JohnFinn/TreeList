#pragma once

#include <iterator>
#include "Node.h"


template <typename T>
class TreeListIterator : std::iterator<std::input_iterator_tag, T> {
    Node<T>* node;
public:
    explicit TreeListIterator(Node<T>* node) noexcept : node(node) {}

    TreeListIterator(const TreeListIterator& other) noexcept;

    TreeListIterator& operator=(const TreeListIterator& other) const noexcept;

    bool operator!=(const TreeListIterator& other) const noexcept;

    bool operator==(const TreeListIterator& other) const noexcept;

    const T& operator*() const noexcept;

    TreeListIterator& operator++() noexcept;
};

template <typename T>
struct std::iterator_traits<TreeListIterator<T> >{
    typedef T value_type;
};
