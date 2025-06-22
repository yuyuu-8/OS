#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <sstream>


struct employee {
    int num;
    char name[10];
    double hours;
};

std::shared_mutex recordMutex[100];
std::string pipeName = R"(\\.\pipe\employee_pipe)";
std::string filename;

void printFile(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        std::cerr << "Ошибка при открытии файла!" << std::endl;
        return;
    }
    employee e;
    std::cout << "Содержимое файла:" << std::endl;
    while (in.read(reinterpret_cast<char*>(&e), sizeof(employee))) {
        std::cout << "ID: " << e.num << ", Name: " << e.name << ", Hours: " << e.hours << std::endl;
    }
}

void startClient(int index) {
    std::string command = "start cmd /k \"Client.exe " + pipeName + "\"";
    system(command.c_str());
}

void handleClient(HANDLE pipe) {
    DWORD bytesRead, bytesWritten;
    char buffer[1024];

    while (true) {
        if (!ReadFile(pipe, buffer, sizeof(buffer), &bytesRead, nullptr) || bytesRead == 0) break;

        std::string request(buffer, bytesRead);
        std::istringstream iss(request);
        std::string command;
        int id;
        iss >> command >> id;

        std::fstream file(filename, std::ios::in | std::ios::out | std::ios::binary);
        if (!file) {
            std::string msg = "Ошибка открытия файла";
            WriteFile(pipe, msg.c_str(), msg.size(), &bytesWritten, nullptr);
            continue;
        }

        bool found = false;
        employee e;
        while (file.read(reinterpret_cast<char*>(&e), sizeof(employee))) {
            if (e.num == id) {
                found = true;
                break;
            }
        }

        if (!found) {
            std::string msg = "Запись не найдена";
            WriteFile(pipe, msg.c_str(), msg.size(), &bytesWritten, nullptr);
            continue;
        }

        if (command == "read") {
            std::shared_lock lock(recordMutex[id]);
            file.clear();
            file.seekg((file.tellg() - static_cast<std::streamoff>(sizeof(employee))));
            file.read(reinterpret_cast<char*>(&e), sizeof(employee));
            WriteFile(pipe, reinterpret_cast<char*>(&e), sizeof(employee), &bytesWritten, nullptr);
            ReadFile(pipe, buffer, sizeof(buffer), &bytesRead, nullptr);
        }
        else if (command == "write") {
            std::unique_lock lock(recordMutex[id]);
            WriteFile(pipe, reinterpret_cast<char*>(&e), sizeof(employee), &bytesWritten, nullptr);
            ReadFile(pipe, reinterpret_cast<char*>(&e), sizeof(employee), &bytesRead, nullptr);
            file.clear();
            file.seekp((file.tellg() - static_cast<std::streamoff>(sizeof(employee))));
            file.write(reinterpret_cast<char*>(&e), sizeof(employee));
            ReadFile(pipe, buffer, sizeof(buffer), &bytesRead, nullptr);
        }
    }
    DisconnectNamedPipe(pipe);
    CloseHandle(pipe);
}

int main() {
    setlocale(LC_ALL, "Russian");
    std::cout << "Введите имя файла: ";
    std::getline(std::cin, filename);

    int n;
    std::cout << "Введите количество сотрудников: ";
    std::cin >> n;

    std::vector<employee> employees(n);
    for (int i = 0; i < n; ++i) {
        std::cout << "Сотрудник " << i + 1 << " (num name hours): ";
        std::cin >> employees[i].num >> employees[i].name >> employees[i].hours;
    }

    std::ofstream out(filename, std::ios::binary);
    for (const auto& e : employees) {
        out.write(reinterpret_cast<const char*>(&e), sizeof(employee));
    }
    out.close();

    printFile(filename);

    int clientCount;
    std::cout << "Введите количество клиентов: ";
    std::cin >> clientCount;
    std::cin.ignore();

    for (int i = 0; i < clientCount; ++i) {
        startClient(i);
    }

    std::vector<std::thread> handlers;

    for (int i = 0; i < clientCount; ++i) {
        HANDLE pipe = CreateNamedPipeA(
            pipeName.c_str(),
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
            clientCount,
            0, 0, 0, nullptr);

        if (pipe == INVALID_HANDLE_VALUE) {
            std::cerr << "Ошибка создания канала" << std::endl;
            return 1;
        }

        std::cout << "Ожидание подключения клиента " << i + 1 << "..." << std::endl;
        ConnectNamedPipe(pipe, nullptr);
        handlers.emplace_back(handleClient, pipe);
    }

    std::string command;
    while (true) {
        std::cout << "Введите 'exit' для завершения: ";
        std::getline(std::cin, command);
        if (command == "exit") break;
    }

    for (auto& t : handlers) {
        if (t.joinable()) t.join();
    }

    printFile(filename);
    return 0;
}
