#include "Fibonacci.h"

Fibonacci::Fibonacci() {}

bool Fibonacci::isValid(int n) {
    if (n <= 0) {
        throw std::invalid_argument("n must be a positive number");
    }
    return true;
}

std::vector<int> Fibonacci::getFirstN(int n) {
    isValid(n);

    std::vector<int> sequence;
    if (n >= 1) sequence.push_back(0);
    if (n >= 2) sequence.push_back(1);

    for (int i = 2; i < n; ++i) {
        sequence.push_back(sequence[i - 1] + sequence[i - 2]);
    }

    return sequence;
}