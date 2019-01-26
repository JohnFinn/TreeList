#include <gtest/gtest.h>
#include <algorithm>
#include <vector>

#include "TreeList.h"

TEST(InputIterator, empty_sequence){
    TreeList<int> list;
    std::vector<int> vec;
    ASSERT_TRUE(std::equal(list.begin(), list.end(), vec.begin()));
}

TEST(InputIterator, one_element){
    TreeList<int> list;
    std::vector<int> vec;
    list.push_back(99);
    vec.push_back(99);
    ASSERT_TRUE(std::equal(list.begin(), list.end(), vec.begin()));
}
