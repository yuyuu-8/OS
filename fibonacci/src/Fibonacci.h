#ifndef FIBONACCI_H
#define FIBONACCI_H

#include <vector>
#include <stdexcept>

class Fibonacci {
public:
    Fibonacci();
    std::vector<int> getFirstN(int n);

private:
    bool isValid(int n);
};

#endif // FIBONACCI_H