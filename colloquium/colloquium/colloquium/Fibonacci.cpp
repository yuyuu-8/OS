#include <iostream>
#include <vector>
#include <stdexcept>

std::vector<int> getFibonacci(int n) {
    if (n <= 0) {
        throw std::invalid_argument("n must be a natural number");
    }

    std::vector<int> fibonacci;
    fibonacci.push_back(0);
    if (n == 1) return fibonacci;

    fibonacci.push_back(1);
    if (n == 2) return fibonacci;

    for (int i = 2; i < n; ++i) {
        fibonacci.push_back(fibonacci[i - 1] + fibonacci[i - 2]);
    }
    return fibonacci;
}
