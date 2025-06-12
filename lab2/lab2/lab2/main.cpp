#include <iostream>
#include <vector>

int main() {
    setlocale(LC_ALL, "Russian");

    int n;
    std::cout << "¬ведите размер массива: ";
    std::cin >> n;

    std::vector<int> arr(n);
    std::cout << "¬ведите " << n << " элементов массива: ";
    for (int i = 0; i < n; ++i) {
        std::cin >> arr[i];
    }

    std::cout << "¬веденный массив: ";
    for (int i = 0; i < n; ++i) {
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;

    return 0;
}