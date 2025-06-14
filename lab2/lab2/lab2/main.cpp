#include <iostream>
#include <vector>
#include <windows.h>
#include <locale>

// Структура для передачи данных в потоки и получения результатов
struct ThreadData {
    std::vector<int>* arr; // Указатель на массив
    int min_val;           // Результат от потока min_max
    int max_val;           // Результат от потока min_max
    double average_val;    // Результат от потока average
};

// Функция для потока min_max
DWORD WINAPI min_max_proc(LPVOID lpParameter) {
    ThreadData* data = static_cast<ThreadData*>(lpParameter);

    if (data->arr->empty()) return 1;

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

    std::cout << "Поток min_max: нашел min = " << minVal << " и max = " << maxVal << std::endl;
    return 0;
}

// Функция для потока average
DWORD WINAPI average_proc(LPVOID lpParameter) {
    ThreadData* data = static_cast<ThreadData*>(lpParameter);

    if (data->arr->empty()) {
        data->average_val = 0;
        return 1;
    }

    double sum = 0;
    for (int val : *data->arr) {
        sum += val;
        Sleep(7);
    }

    data->average_val = sum / data->arr->size();

    std::cout << "Поток average: нашел среднее значение = " << data->average_val << std::endl;
    return 0;
}

int main() {
    setlocale(LC_ALL, "Russian");

    // 1. Создание массива
    int n;
    std::cout << "Введите размер массива: ";
    std::cin >> n;
    if (n <= 0) {
        std::cout << "Размер должен быть положительным." << std::endl;
        return 1;
    }

    std::vector<int> arr(n);
    std::cout << "Введите " << n << " элементов массива: ";
    for (int i = 0; i < n; ++i) {
        std::cin >> arr[i];
    }
    std::cout << std::endl;

    // Подготовка данных для потоков
    ThreadData data;
    data.arr = &arr;

    // 2. Создание потоков
    HANDLE hMinMax = CreateThread(NULL, 0, min_max_proc, &data, 0, NULL);
    HANDLE hAverage = CreateThread(NULL, 0, average_proc, &data, 0, NULL);

    // 3. Ожидание завершения потоков
    WaitForSingleObject(hMinMax, INFINITE);
    WaitForSingleObject(hAverage, INFINITE);

    std::cout << "\nПоток main: дочерние потоки завершены." << std::endl;

    // 4. Замена максимального и минимального элементов
    int min_elem = data.min_val;
    int max_elem = data.max_val;
    int average_to_replace = static_cast<int>(data.average_val); // Приводим к int для замены

    bool min_replaced = false;
    bool max_replaced = false;

    for (int i = 0; i < arr.size(); ++i) {
        if (arr[i] == min_elem && !min_replaced) {
            arr[i] = average_to_replace;
            min_replaced = true;
        }
        if (arr[i] == max_elem && !max_replaced) {
            arr[i] = average_to_replace;
            max_replaced = true;
        }
    }

    std::cout << "Поток main: заменил min и max на среднее значение." << std::endl;
    std::cout << "\nИтоговый массив: ";
    for (int val : arr) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    // Закрытие дескрипторов
    CloseHandle(hMinMax);
    CloseHandle(hAverage);

    // 5. Завершение работы
    return 0;
}