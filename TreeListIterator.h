#pragma once
#include <iterator>

template <typename T>
class TreeListIterator : std::iterator<std::input_iterator_tag, T> {
public:
    TreeListIterator(const TreeListIterator& other) noexcept;

    ~TreeListIterator() noexcept;

    TreeListIterator& operator=(const TreeListIterator& other) noexcept;

    bool operator==(const TreeListIterator& other) const noexcept;

    const T& operator*() const noexcept;

    TreeListIterator& operator++() noexcept;
};
