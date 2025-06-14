#include <iostream>
#include <vector>
#include <windows.h>

struct ThreadData {
    std::vector<int>* arr;
    int min_val;
    int max_val;
    double average_val;
};

// ������� ��� ������ min_max
DWORD WINAPI min_max_proc(LPVOID lpParameter) {
    std::cout << "����� min_max �������." << std::endl;
    ThreadData* data = (ThreadData*)lpParameter;

    if (data->arr->empty()) {
        return 1; // ������, ������ ����
    }

    int minVal = (*data->arr)[0];
    int maxVal = (*data->arr)[0];

    for (size_t i = 1; i < data->arr->size(); ++i) {
        if ((*data->arr)[i] < minVal) {
            minVal = (*data->arr)[i];
        }
        Sleep(7);

        if ((*data->arr)[i] > maxVal) {
            maxVal = (*data->arr)[i];
        }
        Sleep(7);
    }

    data->min_val = minVal;
    data->max_val = maxVal;

    std::cout << "����� min_max: ����������� ������� = " << minVal << std::endl;
    std::cout << "����� min_max: ������������ ������� = " << maxVal << std::endl;

    return 0;
}

// ������� ��� ������ average
DWORD WINAPI average_proc(LPVOID lpParameter) {
    std::cout << "����� average �������." << std::endl;
    // TODO
    return 0;
}

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

    // ���������� ������ ��� �������
    ThreadData data;
    data.arr = &arr;

    // �������� �������
    HANDLE hMinMax = CreateThread(NULL, 0, min_max_proc, &data, 0, NULL);
    HANDLE hAverage = CreateThread(NULL, 0, average_proc, &data, 0, NULL);

    // �������� ���������� �������
    std::cout << "����� main ������� ���������� �������� �������..." << std::endl;
    WaitForSingleObject(hMinMax, INFINITE);
    WaitForSingleObject(hAverage, INFINITE);

    std::cout << "�������� ������ ��������� ������." << std::endl;

    // �������� ������������ �������
    CloseHandle(hMinMax);
    CloseHandle(hAverage);

    std::cout << "\n������ ��������� ���������." << std::endl;

    return 0;
}