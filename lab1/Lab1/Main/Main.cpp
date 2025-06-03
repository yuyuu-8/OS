#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h> // ��� CreateProcess � WaitForSingleObject
#include <iomanip>   // ��� ������
#include "employee.h" // ��� ������ � ������ ��������� �����

// ������� ��� ������� ��������
bool launchProcess(const std::string& commandLine) {
    STARTUPINFOA si; // ���������� ANSI-������ STARTUPINFOA
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // CreateProcess ������� ������������� ������ ��� commandLine
    char cmdLine[512]; // ����� ��� ��������� ������
    strncpy_s(cmdLine, commandLine.c_str(), sizeof(cmdLine) - 1);
    cmdLine[sizeof(cmdLine) - 1] = 0; // ����������� ����-����������

    if (!CreateProcessA(NULL,   // ��� ������ (���������� ��������� ������)
        cmdLine,// ��������� ������
        NULL,   // ���������� ������ ��������
        NULL,   // ���������� ������ ������
        FALSE,  // ������������ ������������
        0,      // ����� ��������
        NULL,   // ������������ ���� ���������
        NULL,   // ������������ ������� ����������
        &si,    // ��������� �� STARTUPINFO
        &pi)    // ��������� �� PROCESS_INFORMATION
        ) {
        std::cerr << "CreateProcess failed (" << GetLastError() << "). Command: " << commandLine << std::endl;
        return false;
    }

    // ���� ���������� ��������� ��������
    WaitForSingleObject(pi.hProcess, INFINITE);

    // ��������� ����������� �������� � ������
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

    // 1. ����������� � ������� ��� ��������� ����� � ���������� �������
    std::cout << "Enter binary file name: ";
    std::cin >> binaryFileName;
    std::cout << "Enter number of records: ";
    std::cin >> numRecords;

    // 2. ��������� ������� Creator
    std::string creatorCmd = "Creator.exe " + binaryFileName + " " + std::to_string(numRecords);
    std::cout << "Launching Creator: " << creatorCmd << std::endl;
    if (!launchProcess(creatorCmd)) {
        std::cerr << "Failed to run Creator.\n";
        return 1;
    }
    std::cout << "Creator finished.\n";

    // 3. ���� ���������� (������� � launchProcess)

    // 4. ������� ���������� ���������� ��������� ����� �� �������
    displayBinaryFile(binaryFileName);

    // 5. ����������� � ������� ��� ����� ������ � ������ �� ��� ������
    std::cout << "Enter report file name: ";
    std::cin >> reportFileName;
    std::cout << "Enter hourly rate: ";
    std::cin >> hourlyRate;

    // 6. ��������� ������� Reporter
    std::string reporterCmd = "Reporter.exe " + binaryFileName + " " + reportFileName + " " + std::to_string(hourlyRate);
    std::cout << "Launching Reporter: " << reporterCmd << std::endl;
    if (!launchProcess(reporterCmd)) {
        std::cerr << "Failed to run Reporter.\n";
        return 1;
    }
    std::cout << "Reporter finished.\n";

    // 7. ���� ���������� (������� � launchProcess)

    // 8. ������� ����� �� �������
    displayTextFile(reportFileName);

    // 9. ��������� ���� ������
    std::cout << "Main program finished.\n";
    return 0;
}