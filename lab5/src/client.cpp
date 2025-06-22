#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>

struct employee {
    int num;
    char name[10];
    double hours;
};

void printEmployee(const employee& e) {
    std::cout << "ID: " << e.num << ", Name: " << e.name << ", Hours: " << e.hours << std::endl;
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");
    if (argc < 2) {
        std::cerr << "Ошибка: не передано имя канала." << std::endl;
        return 1;
    }

    std::string pipeName = argv[1];
    HANDLE pipe;

    while (true) {
        pipe = CreateFileA(
            pipeName.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr
        );

        if (pipe != INVALID_HANDLE_VALUE)
            break;

        if (GetLastError() != ERROR_PIPE_BUSY) {
            std::cerr << "Не удалось подключиться к каналу." << std::endl;
            return 1;
        }

        WaitNamedPipeA(pipeName.c_str(), 5000);
    }

    DWORD bytesWritten, bytesRead;
    char buffer[1024];

    while (true) {
        std::cout << "\nВыберите операцию:\n"
            "1. Читать запись\n"
            "2. Изменить запись\n"
            "3. Выход\n"
            "Введите номер операции: ";
        int choice;
        std::cin >> choice;

        if (choice == 3) break;

        int id;
        std::cout << "Введите ID сотрудника: ";
        std::cin >> id;

        std::ostringstream oss;
        if (choice == 1)
            oss << "read " << id;
        else if (choice == 2)
            oss << "write " << id;
        else
            continue;

        std::string request = oss.str();
        WriteFile(pipe, request.c_str(), request.size(), &bytesWritten, nullptr);

        employee e;
        if (!ReadFile(pipe, &e, sizeof(employee), &bytesRead, nullptr) || bytesRead != sizeof(employee)) {
            std::cerr << "Ошибка чтения или сотрудник не найден.\n";
            continue;
        }

        std::cout << "Полученные данные:\n";
        printEmployee(e);

        if (choice == 1) {
            std::cout << "Нажмите Enter для завершения чтения...";
            std::cin.ignore();
            std::cin.get();
            std::string ok = "done";
            WriteFile(pipe, ok.c_str(), ok.size(), &bytesWritten, nullptr);
        }
        else if (choice == 2) {
            std::cout << "Введите новое имя (до 9 символов): ";
            std::cin >> e.name;
            std::cout << "Введите новое количество часов: ";
            std::cin >> e.hours;

            WriteFile(pipe, &e, sizeof(employee), &bytesWritten, nullptr);
            std::cout << "Нажмите Enter для завершения записи...";
            std::cin.ignore();
            std::cin.get();
            std::string ok = "done";
            WriteFile(pipe, ok.c_str(), ok.size(), &bytesWritten, nullptr);
        }
    }

    CloseHandle(pipe);
    return 0;
}
