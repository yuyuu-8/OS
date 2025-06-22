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
        std::cerr << "������: �� �������� ��� ������." << std::endl;
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
            std::cerr << "�� ������� ������������ � ������." << std::endl;
            return 1;
        }

        WaitNamedPipeA(pipeName.c_str(), 5000);
    }

    DWORD bytesWritten, bytesRead;
    char buffer[1024];

    while (true) {
        std::cout << "\n�������� ��������:\n"
            "1. ������ ������\n"
            "2. �������� ������\n"
            "3. �����\n"
            "������� ����� ��������: ";
        int choice;
        std::cin >> choice;

        if (choice == 3) break;

        int id;
        std::cout << "������� ID ����������: ";
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
            std::cerr << "������ ������ ��� ��������� �� ������.\n";
            continue;
        }

        std::cout << "���������� ������:\n";
        printEmployee(e);

        if (choice == 1) {
            std::cout << "������� Enter ��� ���������� ������...";
            std::cin.ignore();
            std::cin.get();
            std::string ok = "done";
            WriteFile(pipe, ok.c_str(), ok.size(), &bytesWritten, nullptr);
        }
        else if (choice == 2) {
            std::cout << "������� ����� ��� (�� 9 ��������): ";
            std::cin >> e.name;
            std::cout << "������� ����� ���������� �����: ";
            std::cin >> e.hours;

            WriteFile(pipe, &e, sizeof(employee), &bytesWritten, nullptr);
            std::cout << "������� Enter ��� ���������� ������...";
            std::cin.ignore();
            std::cin.get();
            std::string ok = "done";
            WriteFile(pipe, ok.c_str(), ok.size(), &bytesWritten, nullptr);
        }
    }

    CloseHandle(pipe);
    return 0;
}
