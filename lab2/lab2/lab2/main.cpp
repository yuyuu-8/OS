#include <iostream>
#include <vector>
#include <windows.h>
#include <string>
#include <stdexcept>

// Helper function to print detailed error messages for WinAPI calls
void printError(const std::string& functionName, DWORD errorCode) {
    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::cerr << "Error in function '" << functionName << "': " << messageBuffer << " (Error code: " << errorCode << ")" << std::endl;

    LocalFree(messageBuffer); // Free the buffer allocated by FormatMessage
}

// Structure for passing data to threads and receiving results
struct ThreadData {
    std::vector<int>* arr; // Pointer to the array
    int min_val;           // Result from the min_max thread
    int max_val;           // Result from the min_max thread
    double average_val;    // Result from the average thread

    // Constructor
    ThreadData() : arr(nullptr), min_val(0), max_val(0), average_val(0.0) {}
};

// Thread function to find min and max elements
DWORD WINAPI min_max_proc(LPVOID lpParameter) {
    ThreadData* data = static_cast<ThreadData*>(lpParameter);

    if (data->arr->empty()) {
        return 1;
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

    std::cout << "min_max thread: Found min = " << minVal << " and max = " << maxVal << std::endl;
    return 0;
}

// Thread function to calculate the average
DWORD WINAPI average_proc(LPVOID lpParameter) {
    ThreadData* data = static_cast<ThreadData*>(lpParameter);

    if (data->arr->empty()) {
        data->average_val = 0;
        return 1;
    }

    double sum = 0;
    for (int val : *data->arr) {
        sum += val;
        Sleep(12);
    }

    data->average_val = sum / data->arr->size();

    std::cout << "average thread: Found average = " << data->average_val << std::endl;
    return 0;
}

int main() {
    try {
        // 1. Create the array from console input 
        int n;
        std::cout << "Enter array size: ";
        while (!(std::cin >> n) || n <= 0) {
            std::cout << "Invalid input. Please enter a positive integer: ";
            std::cin.clear(); // Clear error flags
            std::cin.ignore(10000, '\n'); // Discard bad input
        }

        std::vector<int> arr(n);
        std::cout << "Enter " << n << " integer elements: ";
        for (int i = 0; i < n; ++i) {
            while (!(std::cin >> arr[i])) {
                std::cout << "Invalid input for element " << i + 1 << ". Please enter an integer: ";
                std::cin.clear();
                std::cin.ignore(10000, '\n');
            }
        }
        std::cout << std::endl;

        // Prepare data for the threads
        ThreadData data;
        data.arr = &arr;

        // 2. Create the min_max and average threads 
        HANDLE hMinMax = CreateThread(NULL, 0, min_max_proc, &data, 0, NULL);
        if (hMinMax == NULL) {
            printError("CreateThread (min_max)", GetLastError());
            throw std::runtime_error("Failed to create min_max thread.");
        }

        HANDLE hAverage = CreateThread(NULL, 0, average_proc, &data, 0, NULL);
        if (hAverage == NULL) {
            printError("CreateThread (average)", GetLastError());
            CloseHandle(hMinMax); // Clean up the first handle
            throw std::runtime_error("Failed to create average thread.");
        }

        // 3. Wait for the threads to complete 
        WaitForSingleObject(hMinMax, INFINITE);
        WaitForSingleObject(hAverage, INFINITE);

        std::cout << "\nmain thread: Child threads have finished execution." << std::endl;

        // 4. Replace the min and max elements with the average value 
        int min_elem = data.min_val;
        int max_elem = data.max_val;
        int average_to_replace = static_cast<int>(data.average_val);

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

        std::cout << "main thread: Replaced min and max elements with the average value." << std::endl;
        std::cout << "\nFinal array: ";
        for (int val : arr) {
            std::cout << val << " ";
        }
        std::cout << std::endl;

        // Clean up thread handles
        CloseHandle(hMinMax);
        CloseHandle(hAverage);

    }
    catch (const std::runtime_error& e) {
        std::cerr << "\nA runtime error occurred: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "\nAn unexpected error occurred." << std::endl;
        return 1;
    }

    // 5. Terminate the process 
    return 0;
}