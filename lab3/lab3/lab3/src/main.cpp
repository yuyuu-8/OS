#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <limits>
#define NOMINMAX
#include <windows.h>
#include "../include/marker_thread.h"
#include <memory>

// Helper function to print array content
void print_array(const std::vector<int>& array) {
    std::cout << "Array content: ";
    for (const auto& element : array) {
        std::cout << element << " ";
    }
    std::cout << std::endl;
}

// Helper function to get and check user input
template<typename T>
T get_validated_input(const std::string& prompt, T min_value, T max_value) {
    T value;
    while (true) {
        std::cout << prompt;
        if (!(std::cin >> value)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            throw InvalidInputException("Invalid input format");
        }
        if (value >= min_value && value <= max_value) {
            return value;
        }
        throw InvalidInputException("Value out of valid range");
    }
}

// Implementation of marker thread function
void marker_func(int id,
                std::vector<int>& array,
                std::vector<bool>& terminate_flags,
                std::mutex& mutex,
                std::condition_variable& start_cv,
                std::condition_variable& main_wait_cv,
                std::condition_variable& marker_wait_cv,
                bool& start_signal,
                int& stuck_markers_count) {
    try {
        // Local storage for marked positions
        std::vector<size_t> marked_positions;
        int marked_elements_count = 0;

        // Wait for start signal
        {
            std::unique_lock<std::mutex> lock(mutex);
            start_cv.wait(lock, [&start_signal] { return start_signal; });
        }

        // Initialize random generator
        std::srand(static_cast<unsigned int>(id + time(nullptr)));

        while (true) {
            {
                std::lock_guard<std::mutex> lock(mutex);
                if (terminate_flags[static_cast<size_t>(id)]) {
                    break;
                }
            }

            // Generate random index
            size_t index = static_cast<size_t>(std::rand()) % array.size();

            std::unique_lock<std::mutex> lock(mutex);

            if (array[index] == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                array[index] = id;
                marked_positions.push_back(index);
                marked_elements_count++;
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
            else {
                std::cout << "\nThread #" << id << " cannot mark element at index " << index << "." << std::endl;
                std::cout << "Thread #" << id << " has marked " << marked_elements_count << " elements." << std::endl;

                stuck_markers_count++;
                main_wait_cv.notify_one();
                marker_wait_cv.wait(lock);
            }
        }

        // Cleanup marked positions
        std::cout << "Thread #" << id << " received termination signal. Cleaning up..." << std::endl;
        {
            std::lock_guard<std::mutex> lock(mutex);
            for (const auto& pos : marked_positions) {
                if (array[pos] == id) {
                    array[pos] = 0;
                }
            }
        }
        std::cout << "Thread #" << id << " finished." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error in thread #" << id << ": " << e.what() << std::endl;
        throw;
    }
}

int main() {
    try {
        // Set console code page to UTF-8
        if (!SetConsoleOutputCP(CP_UTF8)) {
            throw MarkerThreadException("Failed to set console code page: " +
                std::to_string(static_cast<int>(GetLastError())));
        }

        // Get array size
        size_t array_size = static_cast<size_t>(
            get_validated_input<int>("Enter array size: ", 1, 1000));

        // Create and initialize array
        std::vector<int> array(array_size, 0);

        // Get number of marker threads
        int marker_count = get_validated_input<int>(
            "Enter number of marker threads: ", 1, 100);

        // Initialize synchronization primitives
        std::mutex mutex;
        std::condition_variable start_cv, main_wait_cv, marker_wait_cv;
        bool start_signal = false;
        int stuck_markers_count = 0;

        // Initialize thread control variables
        std::vector<bool> terminate_flags(static_cast<size_t>(marker_count + 1), false);
        std::vector<bool> active_threads(static_cast<size_t>(marker_count + 1), true);
        int active_thread_count = marker_count;

        // Create and start marker threads
        std::vector<std::thread> threads;
        threads.reserve(static_cast<size_t>(marker_count));

        for (int i = 1; i <= marker_count; ++i) {
            threads.emplace_back(marker_func, i, std::ref(array),
                std::ref(terminate_flags), std::ref(mutex),
                std::ref(start_cv), std::ref(main_wait_cv),
                std::ref(marker_wait_cv), std::ref(start_signal),
                std::ref(stuck_markers_count));
        }
        std::cout << marker_count << " marker threads created." << std::endl;

        // Wait for threads to initialize
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Send start signal
        std::cout << "\n--- Sending START signal to all markers ---" << std::endl;
        {
            std::lock_guard<std::mutex> lock(mutex);
            start_signal = true;
        }
        start_cv.notify_all();

        // Main work cycle
        while (active_thread_count > 0) {
            // Wait for all threads to get stuck
            {
                std::unique_lock<std::mutex> lock(mutex);
                main_wait_cv.wait(lock, [&] {
                    return stuck_markers_count == active_thread_count;
                });
            }

            std::cout << "\n--- All active threads are stuck. ---" << std::endl;
            print_array(array);

            // Get thread number to terminate
            int thread_to_terminate;
            try {
                thread_to_terminate = get_validated_input<int>(
                    "Enter marker number to terminate: ", 1, marker_count);

                if (!active_threads[static_cast<size_t>(thread_to_terminate)]) {
                    throw InvalidInputException("Thread already terminated");
                }
            }
            catch (const InvalidInputException& e) {
                std::cout << "Error: " << e.what() << ". Try again." << std::endl;
                continue;
            }

            // Send termination signal
            {
                std::lock_guard<std::mutex> lock(mutex);
                terminate_flags[static_cast<size_t>(thread_to_terminate)] = true;
            }
            marker_wait_cv.notify_all();

            // Wait for thread termination
            if (threads[static_cast<size_t>(thread_to_terminate - 1)].joinable()) {
                threads[static_cast<size_t>(thread_to_terminate - 1)].join();
            }

            active_threads[static_cast<size_t>(thread_to_terminate)] = false;
            active_thread_count--;

            std::cout << "\n--- Thread #" << thread_to_terminate <<
                        " has been terminated. ---" << std::endl;
            print_array(array);

            if (active_thread_count > 0) {
                // Reset stuck count and continue remaining threads
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    stuck_markers_count = 0;
                }
                std::cout << "--- Sending CONTINUE signal to remaining threads ---" << std::endl;
                marker_wait_cv.notify_all();
            }
        }

        std::cout << "\nAll marker threads have been terminated." << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}