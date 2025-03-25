#include <iostream>
#include <string>
#include <algorithm>

bool isPalindrome(int number) {
    std::string str = std::to_string(number);
    std::string reversed = str;
    std::reverse(reversed.begin(), reversed.end());
    return str == reversed;
}