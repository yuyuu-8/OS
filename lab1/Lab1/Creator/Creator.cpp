#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <limits>
#include "employee.h"

// Function to read an integer
int readInteger(const std::string& prompt) {
    int value;
    while (true) {
        std::cout << prompt;
        std::cin >> value;
        if (std::cin.good()) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clean up buffer
            return value;
        }
        else {
            std::cout << "Error: Invalid input. Please enter a valid integer." << std::endl;
            std::cin.clear(); // Clear error flags
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard bad input
        }
    }
}

// Function to read a double
double readDouble(const std::string& prompt) {
    double value;
    while (true) {
        std::cout << prompt;
        std::cin >> value;
        if (std::cin.good()) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clean up buffer
            return value;
        }
        else {
            std::cout << "Error: Invalid input. Please enter a valid double." << std::endl;
            std::cin.clear(); // Clear error flags
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard bad input
        }
    }
}

// Function to read a string
void readEmployeeName(const std::string& prompt, char* buffer, int bufferSize) {
    while (true) {
        std::cout << prompt;
        // std::cin.getline will read up to bufferSize-1 characters and null-terminate
        std::cin.getline(buffer, bufferSize);

        if (std::cin.good()) {
            if (buffer[0] == '\0') {
                std::cout << "Warning: Name is empty. Proceeding..." << std::endl;
            }
            return;
        }
        else if (std::cin.fail() && !std::cin.eof()) {
            // This case might happen if the line was too long for the buffer
            // cin.getline sets failbit if it reads bufferSize-1 chars but doesn't find newline
            std::cout << "Warning: Name might have been truncated to " << (bufferSize - 1) << " characters." << std::endl;
            std::cin.clear(); // Clear failbit
            // If failbit was due to too long line, the rest of the line is still in the input buffer.
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return; // Return with potentially truncated name
        }
    }
}


int main(int argc, char* argv[]) {
    // 1. Argument validation
    if (argc != 3) {
        std::cerr << "Usage: Creator.exe <output_binary_file> <num_records>" << std::endl;
        std::cerr << "Error: Incorrect number of arguments." << std::endl;
        return 1;
    }

    const char* outputFileName = argv[1];
    int numRecords;

    try {
        numRecords = std::stoi(argv[2]);
        if (numRecords <= 0) {
            throw std::invalid_argument("Number of records must be a positive integer.");
        }
    }
    catch (const std::invalid_argument& e) {
        std::cerr << "Error: Invalid number of records argument: " << e.what() << std::endl;
        return 2;
    }
    catch (const std::out_of_range& e) {
        std::cerr << "Error: Number of records is out of range: " << e.what() << std::endl;
        return 2;
    }

    // 2. File opening
    // Open in binary mode, truncate if exists (overwrite), create if not.
    std::ofstream outFile(outputFileName, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file '" << outputFileName << "' for writing." << std::endl;
        return 3;
    }

    std::cout << "Creator: Preparing to write " << numRecords << " records to '" << outputFileName << "'" << std::endl;

    // 3. Data input and writing
    try {
        for (int i = 0; i < numRecords; ++i) {
            Employee emp;
            std::cout << "\n--- Enter data for employee " << i + 1 << " ---" << std::endl;

            emp.id = readInteger("ID (integer): ");

            readEmployeeName("Name (max " + std::to_string(MAX_EMPLOYEE_NAME_LENGTH) + " chars): ", emp.name, sizeof(emp.name));

            emp.hoursWorked = readDouble("Hours worked (double): ");
            if (emp.hoursWorked < 0) {
                std::cout << "Warning: Negative hours worked entered. Proceeding, but this might be an error." << std::endl;
            }

            outFile.write(reinterpret_cast<const char*>(&emp), sizeof(Employee));

            if (outFile.fail()) {
                throw std::runtime_error("Failed to write record to file. Disk full or other I/O error?");
            }
            std::cout << "Record " << i + 1 << " written." << std::endl;
        }
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Runtime Error during data input/writing: " << e.what() << std::endl;
        outFile.close();
        return 4;
    }

    outFile.close();
    std::cout << "\nCreator: Binary file '" << outputFileName << "' created successfully with " << numRecords << " records." << std::endl;
    return 0;
}