#include "gtest/gtest.h"
#include "Fibonacci.cpp"

TEST(FibonacciTest, TestValidInput) {
    auto result = getFibonacci(5);
    EXPECT_EQ(result, (std::vector<int>{0, 1, 1, 2, 3}));
}

TEST(FibonacciTest, TestInvalidInput) {
    EXPECT_THROW(getFibonacci(0), std::invalid_argument);
}
