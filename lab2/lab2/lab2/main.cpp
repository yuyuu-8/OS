#include <iostream>
#include <vector>

int main() {
    setlocale(LC_ALL, "Russian");

    int n;
    std::cout << "������� ������ �������: ";
    std::cin >> n;

    std::vector<int> arr(n);
    std::cout << "������� " << n << " ��������� �������: ";
    for (int i = 0; i < n; ++i) {
        std::cin >> arr[i];
    }

    std::cout << "��������� ������: ";
    for (int i = 0; i < n; ++i) {
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;

    return 0;
}