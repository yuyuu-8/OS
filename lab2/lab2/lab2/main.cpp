#include <iostream>
#include <vector>
#include <windows.h>

struct ThreadData {
    std::vector<int>* arr;
    int min_val;
    int max_val;
    double average_val;
};

// Функция для потока min_max
DWORD WINAPI min_max_proc(LPVOID lpParameter) {
    std::cout << "Поток min_max запущен." << std::endl;
    ThreadData* data = (ThreadData*)lpParameter;

    if (data->arr->empty()) {
        return 1; // Ошибка, массив пуст
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

    std::cout << "Поток min_max: Минимальный элемент = " << minVal << std::endl;
    std::cout << "Поток min_max: Максимальный элемент = " << maxVal << std::endl;

    return 0;
}

// Функция для потока average
DWORD WINAPI average_proc(LPVOID lpParameter) {
    std::cout << "Поток average запущен." << std::endl;
    // TODO
    return 0;
}

int main() {
    setlocale(LC_ALL, "Russian");

    int n;
    std::cout << "Введите размер массива: ";
    std::cin >> n;

    std::vector<int> arr(n);
    std::cout << "Введите " << n << " элементов массива: ";
    for (int i = 0; i < n; ++i) {
        std::cin >> arr[i];
    }

    // Подготовка данных для потоков
    ThreadData data;
    data.arr = &arr;

    // Создание потоков
    HANDLE hMinMax = CreateThread(NULL, 0, min_max_proc, &data, 0, NULL);
    HANDLE hAverage = CreateThread(NULL, 0, average_proc, &data, 0, NULL);

    // Ожидание завершения потоков
    std::cout << "Поток main ожидает завершения дочерних потоков..." << std::endl;
    WaitForSingleObject(hMinMax, INFINITE);
    WaitForSingleObject(hAverage, INFINITE);

    std::cout << "Дочерние потоки завершили работу." << std::endl;

    // Закрытие дескрипторов потоков
    CloseHandle(hMinMax);
    CloseHandle(hAverage);

    std::cout << "\nРабота программы завершена." << std::endl;

    return 0;
}