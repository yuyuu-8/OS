#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>
#include <iomanip>
#include "employee.h"

// Function to launch a process
bool launchProcess(const std::string& commandLine) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // CreateProcess requires a non-const string for commandLine
    char cmdLine[512]; // Buffer for the command line
    strncpy_s(cmdLine, commandLine.c_str(), sizeof(cmdLine) - 1);
    cmdLine[sizeof(cmdLine) - 1] = 0; // Ensure null-termination

    if (!CreateProcessA(NULL,   // Module name (use command line)
        cmdLine,                // Command line
        NULL,                   // Process security descriptor
        NULL,                   // Thread security descriptor
        FALSE,                  // Handle inheritance
        0,                      // Creation flags
        NULL,                   // Parent environment block
        NULL,                   // Parent current directory
        &si,                    // Pointer to STARTUPINFO
        &pi)                    // Pointer to PROCESS_INFORMATION
        ) {
        std::cerr << "CreateProcess failed (" << GetLastError() << "). Command: " << commandLine << std::endl;
        return false;
    }

    // Wait for the child process to finish
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles
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

    // 1. Ask for the binary file name and number of records from the console
    std::cout << "Enter binary file name: ";
    std::cin >> binaryFileName;
    std::cout << "Enter number of records: ";
    std::cin >> numRecords;

    // 2. Launch the Creator utility
    std::string creatorCmd = "Creator.exe " + binaryFileName + " " + std::to_string(numRecords);
    std::cout << "Launching Creator: " << creatorCmd << std::endl;
    if (!launchProcess(creatorCmd)) {
        std::cerr << "Failed to run Creator.\n";
        return 1;
    }
    std::cout << "Creator finished.\n";

    // 3. Wait for it to finish

    // 4. Display the contents of the created binary file to the console
    displayBinaryFile(binaryFileName);

    // 5. Ask for the report file name and hourly wage from the console
    std::cout << "Enter report file name: ";
    std::cin >> reportFileName;
    std::cout << "Enter hourly rate: ";
    std::cin >> hourlyRate;

    // 6. Launch the Reporter utility
    std::string reporterCmd = "Reporter.exe " + binaryFileName + " " + reportFileName + " " + std::to_string(hourlyRate);
    std::cout << "Launching Reporter: " << reporterCmd << std::endl;
    if (!launchProcess(reporterCmd)) {
        std::cerr << "Failed to run Reporter.\n";
        return 1;
    }
    std::cout << "Reporter finished.\n";

    // 7. Wait for it to finish

    // 8. Display the report to the console
    displayTextFile(reportFileName);

    // 9. End the main program
    std::cout << "Main program finished.\n";
    return 0;
}
