#include "gtest/gtest.h"
#include "LinkedList.cpp"

TEST(LinkedListTest, TestReverseList) {
    auto head = std::make_shared<Node>(1);
    head->next = std::make_shared<Node>(2);
    head->next->next = std::make_shared<Node>(3);
    head->next->next->next = std::make_shared<Node>(4);

    auto reversed = reverseList(head);
    std::vector<int> expected = { 4, 3, 2, 1 };
    int i = 0;
    while (reversed != nullptr) {
        EXPECT_EQ(reversed->data, expected[i++]);
        reversed = reversed->next;
    }
}
