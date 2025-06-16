// ФИНАЛЬНАЯ ВЕРСИЯ
// main.cpp

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <cstdlib>

// --- Общие ресурсы ---
std::vector<int>* g_array = nullptr; // Указатель на общий массив
int g_array_size = 0;                // Размер массива

std::vector<bool> g_terminate_flags; // Флаги для сигнала на завершение каждому потоку

// --- Примитивы синхронизации ---
std::mutex g_mutex;                      // Главный мьютекс для защиты ВСЕХ общих данных
std::condition_variable g_start_cv;      // Условная переменная для одновременного старта
bool g_start_signal = false;             // Предикат для g_start_cv

std::condition_variable g_main_wait_cv;  // УВ, на которой ждет main, пока все маркеры не застрянут
std::condition_variable g_marker_wait_cv;// УВ, на которой ждут маркеры сигнала от main

int g_stuck_markers_count = 0;           // Счетчик "застрявших" потоков

// --- Вспомогательная функция для вывода массива ---
void print_array() {
    std::cout << "Array content: ";
    for (int i = 0; i < g_array_size; ++i) {
        std::cout << (*g_array)[i] << " ";
    }
    std::cout << std::endl;
}

// --- Функция потока marker ---
// --- Функция потока marker (ИСПРАВЛЕННАЯ И БОЛЕЕ НАДЕЖНАЯ ВЕРСИЯ) ---
void marker_func(int id) {
    {
        std::unique_lock<std::mutex> lock(g_mutex);
        g_start_cv.wait(lock, [] { return g_start_signal; });
    }
    srand(id + time(0));
    int marked_elements_count = 0;

    while (true) {
        {
            std::lock_guard<std::mutex> lock(g_mutex);
            if (g_terminate_flags[id]) {
                break;
            }
        }

        int index = rand() % g_array_size;

        // Блокируем мьютекс НА ВСЕ ВРЕМЯ ОПЕРАЦИИ
        std::unique_lock<std::mutex> lock(g_mutex);

        if ((*g_array)[index] == 0) {
            // Если нашли свободное место, просто и надежно его занимаем.
            // Без лишних снов и освобождений мьютекса.

            // Задание требует спать 5 мс до и после. Выполним это,
            // но будем держать мьютекс, чтобы избежать гонок.
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            (*g_array)[index] = id;
            marked_elements_count++;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        else {
            // Если место занято, сообщаем main и ждем.
            std::cout << "\nThread #" << id << " cannot mark element at index " << index << "." << std::endl;
            std::cout << "Thread #" << id << " has marked " << marked_elements_count << " elements." << std::endl;

            g_stuck_markers_count++;
            g_main_wait_cv.notify_one();
            g_marker_wait_cv.wait(lock);
        }
        // unique_lock автоматически освободит мьютекс здесь
    }

    // ... остальная часть функции без изменений ...
    std::cout << "Thread #" << id << " received termination signal. Cleaning up..." << std::endl;
    std::lock_guard<std::mutex> lock(g_mutex);
    for (int i = 0; i < g_array_size; ++i) {
        if ((*g_array)[i] == id) {
            (*g_array)[i] = 0;
        }
    }
    std::cout << "Thread #" << id << " finished." << std::endl;
}

// --- Основной поток ---
int main() {
    setlocale(LC_ALL, "Russian");

    // 1. Захват памяти под массив
    std::cout << "Введите размер массива: ";
    std::cin >> g_array_size;
    g_array = new std::vector<int>(g_array_size, 0); // 2. Инициализация нулями

    // 3. Запрос количества потоков
    int marker_count;
    std::cout << "Введите количество потоков marker: ";
    std::cin >> marker_count;

    std::vector<std::thread> threads;
    // +1 в размере, чтобы использовать индексы 1..marker_count для удобства
    g_terminate_flags.resize(marker_count + 1, false);
    std::vector<bool> active_threads(marker_count + 1, true);
    int active_thread_count = marker_count;

    // 4. Запуск потоков marker
    for (int i = 1; i <= marker_count; ++i) {
        threads.emplace_back(marker_func, i);
    }
    std::cout << marker_count << " marker threads created." << std::endl;

    // Небольшая пауза, чтобы все потоки успели создаться и дойти до ожидания
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 5. Сигнал на начало работы всем потокам
    std::cout << "\n--- Sending START signal to all markers ---" << std::endl;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_start_signal = true;
    }
    g_start_cv.notify_all();

    // 6. Основной цикл работы main
    while (active_thread_count > 0) {
        // 6.1. Ждем, пока все активные потоки не застрянут
        {
            std::unique_lock<std::mutex> lock(g_mutex);
            g_main_wait_cv.wait(lock, [&] { return g_stuck_markers_count == active_thread_count; });
        }

        std::cout << "\n--- All active threads are stuck. ---" << std::endl;
        // 6.2. Вывод содержимого массива
        print_array();

        // 6.3. Запрос номера потока на завершение
        int thread_to_terminate;
        while (true) {
            std::cout << "Enter marker number to terminate: ";
            std::cin >> thread_to_terminate;
            if (thread_to_terminate > 0 && thread_to_terminate <= marker_count && active_threads[thread_to_terminate]) {
                break;
            }
            std::cout << "Invalid number or thread already terminated. Try again." << std::endl;
        }

        // 6.4. Подача сигнала на завершение
        {
            std::lock_guard<std::mutex> lock(g_mutex);
            g_terminate_flags[thread_to_terminate] = true;
        }

        // "Разбудим" только тот поток, который должен завершиться.
        // Хотя notify_all() проще и тоже сработает, так как остальные продолжат ждать.
        g_marker_wait_cv.notify_all();

        // 6.5. Ожидание завершения потока
        threads[thread_to_terminate - 1].join();
        active_threads[thread_to_terminate] = false;
        active_thread_count--;

        std::cout << "\n--- Thread #" << thread_to_terminate << " has been terminated. ---" << std::endl;

        // 6.6. Вывод содержимого массива
        print_array();

        if (active_thread_count > 0) {
            // 6.7. Подача сигнала на продолжение работы оставшимся
            {
                std::lock_guard<std::mutex> lock(g_mutex);
                g_stuck_markers_count = 0;
            }
            std::cout << "--- Sending CONTINUE signal to remaining threads ---" << std::endl;
            // Будим все потоки, чтобы они снова попытались работать
            g_marker_wait_cv.notify_all();
        }
    }

    std::cout << "\nAll marker threads have been terminated." << std::endl;
    // 7. Завершение работы
    delete g_array;
    return 0;
}