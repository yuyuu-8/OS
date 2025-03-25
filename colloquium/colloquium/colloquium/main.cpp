#include <iostream>
#include <stdexcept>
#include "Fibonacci.cpp"
#include "Palindrome.cpp"
#include "LinkedList.cpp"

int main() {
    try {
        int n;
        std::cout << "Input n for Fibonacci sequence: ";
        std::cin >> n;
        auto fibonacci = getFibonacci(n);
        std::cout << "First " << n << " Fibonacci numbers: ";
        for (const auto& num : fibonacci) {
            std::cout << num << " ";
        }
        std::cout << std::endl;

        int number;
        std::cout << "Input number to check if it's palindrome: ";
        std::cin >> number;
        if (isPalindrome(number)) {
            std::cout << number << " is palindrome." << std::endl;
        }
        else {
            std::cout << number << " is not a palindrome." << std::endl;
        }

        auto head = std::make_shared<Node>(1);
        head->next = std::make_shared<Node>(2);
        head->next->next = std::make_shared<Node>(3);
        head->next->next->next = std::make_shared<Node>(4);

        std::cout << "List: ";
        printList(head);

        auto reversed = reverseList(head);
        std::cout << "New list: ";
        printList(reversed);
    }
    catch (const std::exception& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
    }

    return 0;
}
