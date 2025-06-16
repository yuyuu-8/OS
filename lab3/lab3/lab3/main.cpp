// ��������� ������
// main.cpp

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <cstdlib>

// --- ����� ������� ---
std::vector<int>* g_array = nullptr; // ��������� �� ����� ������
int g_array_size = 0;                // ������ �������

std::vector<bool> g_terminate_flags; // ����� ��� ������� �� ���������� ������� ������

// --- ��������� ������������� ---
std::mutex g_mutex;                      // ������� ������� ��� ������ ���� ����� ������
std::condition_variable g_start_cv;      // �������� ���������� ��� �������������� ������
bool g_start_signal = false;             // �������� ��� g_start_cv

std::condition_variable g_main_wait_cv;  // ��, �� ������� ���� main, ���� ��� ������� �� ���������
std::condition_variable g_marker_wait_cv;// ��, �� ������� ���� ������� ������� �� main

int g_stuck_markers_count = 0;           // ������� "����������" �������

// --- ��������������� ������� ��� ������ ������� ---
void print_array() {
    std::cout << "Array content: ";
    for (int i = 0; i < g_array_size; ++i) {
        std::cout << (*g_array)[i] << " ";
    }
    std::cout << std::endl;
}

// --- ������� ������ marker ---
// --- ������� ������ marker (������������ � ����� �������� ������) ---
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

        // ��������� ������� �� ��� ����� ��������
        std::unique_lock<std::mutex> lock(g_mutex);

        if ((*g_array)[index] == 0) {
            // ���� ����� ��������� �����, ������ � ������� ��� ��������.
            // ��� ������ ���� � ������������ ��������.

            // ������� ������� ����� 5 �� �� � �����. �������� ���,
            // �� ����� ������� �������, ����� �������� �����.
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            (*g_array)[index] = id;
            marked_elements_count++;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        else {
            // ���� ����� ������, �������� main � ����.
            std::cout << "\nThread #" << id << " cannot mark element at index " << index << "." << std::endl;
            std::cout << "Thread #" << id << " has marked " << marked_elements_count << " elements." << std::endl;

            g_stuck_markers_count++;
            g_main_wait_cv.notify_one();
            g_marker_wait_cv.wait(lock);
        }
        // unique_lock ������������� ��������� ������� �����
    }

    // ... ��������� ����� ������� ��� ��������� ...
    std::cout << "Thread #" << id << " received termination signal. Cleaning up..." << std::endl;
    std::lock_guard<std::mutex> lock(g_mutex);
    for (int i = 0; i < g_array_size; ++i) {
        if ((*g_array)[i] == id) {
            (*g_array)[i] = 0;
        }
    }
    std::cout << "Thread #" << id << " finished." << std::endl;
}

// --- �������� ����� ---
int main() {
    setlocale(LC_ALL, "Russian");

    // 1. ������ ������ ��� ������
    std::cout << "������� ������ �������: ";
    std::cin >> g_array_size;
    g_array = new std::vector<int>(g_array_size, 0); // 2. ������������� ������

    // 3. ������ ���������� �������
    int marker_count;
    std::cout << "������� ���������� ������� marker: ";
    std::cin >> marker_count;

    std::vector<std::thread> threads;
    // +1 � �������, ����� ������������ ������� 1..marker_count ��� ��������
    g_terminate_flags.resize(marker_count + 1, false);
    std::vector<bool> active_threads(marker_count + 1, true);
    int active_thread_count = marker_count;

    // 4. ������ ������� marker
    for (int i = 1; i <= marker_count; ++i) {
        threads.emplace_back(marker_func, i);
    }
    std::cout << marker_count << " marker threads created." << std::endl;

    // ��������� �����, ����� ��� ������ ������ ��������� � ����� �� ��������
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 5. ������ �� ������ ������ ���� �������
    std::cout << "\n--- Sending START signal to all markers ---" << std::endl;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_start_signal = true;
    }
    g_start_cv.notify_all();

    // 6. �������� ���� ������ main
    while (active_thread_count > 0) {
        // 6.1. ����, ���� ��� �������� ������ �� ���������
        {
            std::unique_lock<std::mutex> lock(g_mutex);
            g_main_wait_cv.wait(lock, [&] { return g_stuck_markers_count == active_thread_count; });
        }

        std::cout << "\n--- All active threads are stuck. ---" << std::endl;
        // 6.2. ����� ����������� �������
        print_array();

        // 6.3. ������ ������ ������ �� ����������
        int thread_to_terminate;
        while (true) {
            std::cout << "Enter marker number to terminate: ";
            std::cin >> thread_to_terminate;
            if (thread_to_terminate > 0 && thread_to_terminate <= marker_count && active_threads[thread_to_terminate]) {
                break;
            }
            std::cout << "Invalid number or thread already terminated. Try again." << std::endl;
        }

        // 6.4. ������ ������� �� ����������
        {
            std::lock_guard<std::mutex> lock(g_mutex);
            g_terminate_flags[thread_to_terminate] = true;
        }

        // "��������" ������ ��� �����, ������� ������ �����������.
        // ���� notify_all() ����� � ���� ���������, ��� ��� ��������� ��������� �����.
        g_marker_wait_cv.notify_all();

        // 6.5. �������� ���������� ������
        threads[thread_to_terminate - 1].join();
        active_threads[thread_to_terminate] = false;
        active_thread_count--;

        std::cout << "\n--- Thread #" << thread_to_terminate << " has been terminated. ---" << std::endl;

        // 6.6. ����� ����������� �������
        print_array();

        if (active_thread_count > 0) {
            // 6.7. ������ ������� �� ����������� ������ ����������
            {
                std::lock_guard<std::mutex> lock(g_mutex);
                g_stuck_markers_count = 0;
            }
            std::cout << "--- Sending CONTINUE signal to remaining threads ---" << std::endl;
            // ����� ��� ������, ����� ��� ����� ���������� ��������
            g_marker_wait_cv.notify_all();
        }
    }

    std::cout << "\nAll marker threads have been terminated." << std::endl;
    // 7. ���������� ������
    delete g_array;
    return 0;
}