#include "gtest/gtest.h"
#include "Palindrome.cpp"

TEST(PalindromeTest, TestPalindrome) {
    EXPECT_TRUE(isPalindrome(121));
    EXPECT_TRUE(isPalindrome(12321));
}

TEST(PalindromeTest, TestNonPalindrome) {
    EXPECT_FALSE(isPalindrome(123));
    EXPECT_FALSE(isPalindrome(12345));
}
