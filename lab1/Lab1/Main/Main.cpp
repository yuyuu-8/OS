#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h> // Для CreateProcess и WaitForSingleObject
#include <iomanip>   // Для вывода
#include "employee.h" // Для чтения и вывода бинарного файла

// Функция для запуска процесса
bool launchProcess(const std::string& commandLine) {
    STARTUPINFOA si; // Используем ANSI-версию STARTUPINFOA
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // CreateProcess требует неконстантную строку для commandLine
    char cmdLine[512]; // Буфер для командной строки
    strncpy_s(cmdLine, commandLine.c_str(), sizeof(cmdLine) - 1);
    cmdLine[sizeof(cmdLine) - 1] = 0; // Гарантируем нуль-терминацию

    if (!CreateProcessA(NULL,   // Имя модуля (используем командную строку)
        cmdLine,// Командная строка
        NULL,   // Дескриптор защиты процесса
        NULL,   // Дескриптор защиты потока
        FALSE,  // Наследование дескрипторов
        0,      // Флаги создания
        NULL,   // Родительский блок окружения
        NULL,   // Родительская текущая директория
        &si,    // Указатель на STARTUPINFO
        &pi)    // Указатель на PROCESS_INFORMATION
        ) {
        std::cerr << "CreateProcess failed (" << GetLastError() << "). Command: " << commandLine << std::endl;
        return false;
    }

    // Ждем завершения дочернего процесса
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Закрываем дескрипторы процесса и потока
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}

void displayBinaryFile(const std::string& fileName) {
    std::ifstream inFile(fileName, std::ios::binary | std::ios::in);
    if (!inFile) {
        std::cerr << "Error opening binary file " << fileName << " for reading.\n";
        return;
    }
    std::cout << "\n--- Contents of binary file: " << fileName << " ---\n";
    Employee emp;
    int count = 0;
    while (inFile.read(reinterpret_cast<char*>(&emp), sizeof(Employee))) {
        count++;
        std::cout << "Record " << count << ": ID=" << emp.id
            << ", Name=" << emp.name
            << ", Hours=" << std::fixed << std::setprecision(2) << emp.hoursWorked << std::endl;
    }
    inFile.close();
    if (count == 0) {
        std::cout << "Binary file is empty or could not be read correctly.\n";
    }
    std::cout << "--- End of binary file ---\n\n";
}

void displayTextFile(const std::string& fileName) {
    std::ifstream inFile(fileName);
    if (!inFile) {
        std::cerr << "Error opening text file " << fileName << " for reading.\n";
        return;
    }
    std::cout << "\n--- Contents of report file: " << fileName << " ---\n";
    std::string line;
    while (std::getline(inFile, line)) {
        std::cout << line << std::endl;
    }
    inFile.close();
    std::cout << "--- End of report file ---\n\n";
}


int main() {
    std::string binaryFileName, reportFileName;
    int numRecords;
    double hourlyRate;

    // 1. Запрашиваем с консоли имя бинарного файла и количество записей
    std::cout << "Enter binary file name: ";
    std::cin >> binaryFileName;
    std::cout << "Enter number of records: ";
    std::cin >> numRecords;

    // 2. Запускаем утилиту Creator
    std::string creatorCmd = "Creator.exe " + binaryFileName + " " + std::to_string(numRecords);
    std::cout << "Launching Creator: " << creatorCmd << std::endl;
    if (!launchProcess(creatorCmd)) {
        std::cerr << "Failed to run Creator.\n";
        return 1;
    }
    std::cout << "Creator finished.\n";

    // 3. Ждем завершения (сделано в launchProcess)

    // 4. Выводим содержимое созданного бинарного файла на консоль
    displayBinaryFile(binaryFileName);

    // 5. Запрашиваем с консоли имя файла отчета и оплату за час работы
    std::cout << "Enter report file name: ";
    std::cin >> reportFileName;
    std::cout << "Enter hourly rate: ";
    std::cin >> hourlyRate;

    // 6. Запускаем утилиту Reporter
    std::string reporterCmd = "Reporter.exe " + binaryFileName + " " + reportFileName + " " + std::to_string(hourlyRate);
    std::cout << "Launching Reporter: " << reporterCmd << std::endl;
    if (!launchProcess(reporterCmd)) {
        std::cerr << "Failed to run Reporter.\n";
        return 1;
    }
    std::cout << "Reporter finished.\n";

    // 7. Ждем завершения (сделано в launchProcess)

    // 8. Выводим отчет на консоль
    displayTextFile(reportFileName);

    // 9. Завершаем свою работу
    std::cout << "Main program finished.\n";
    return 0;
}