#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm> // For std::sort
#include <iomanip>   // For std::setw, std::fixed, std::setprecision
#include <stdexcept> // For std::invalid_argument, std::runtime_error
#include "employee.h"

// Comparison function for sorting Employees by ID
bool compareEmployeesById(const Employee& a, const Employee& b) {
    return a.id < b.id;
}

int main(int argc, char* argv[]) {
    // 1. Argument validation
    if (argc != 4) {
        std::cerr << "Usage: Reporter.exe <input_binary_file> <output_report_file> <hourly_rate>" << std::endl;
        std::cerr << "Error: Incorrect number of arguments." << std::endl;
        return 1;
    }

    const char* binaryInputFileName = argv[1];
    const char* reportOutputFileName = argv[2];
    double hourlyRate;

    try {
        // C++ style string to double conversion
        hourlyRate = std::stod(argv[3]);
        if (hourlyRate <= 0) {
            throw std::invalid_argument("Hourly rate must be a positive value.");
        }
    }
    catch (const std::invalid_argument& e) {
        std::cerr << "Error: Invalid hourly rate argument: " << e.what() << std::endl;
        return 2;
    }
    catch (const std::out_of_range& e) {
        std::cerr << "Error: Hourly rate is out of range: " << e.what() << std::endl;
        return 2;
    }

    // 2. Open and read binary input file
    std::ifstream inFile(binaryInputFileName, std::ios::binary | std::ios::in);
    if (!inFile.is_open()) {
        std::cerr << "Error: Could not open binary file '" << binaryInputFileName << "' for reading." << std::endl;
        return 3;
    }

    std::vector<Employee> employees;
    Employee empBuffer;
    try {
        // C++ style cast: reinterpret_cast for reading raw bytes
        while (inFile.read(reinterpret_cast<char*>(&empBuffer), sizeof(Employee))) {
            employees.push_back(empBuffer);
        }
        // Check for read errors that didn't necessarily stop the loop but set fail bits (e.g. partial read at EOF)
        if (inFile.fail() && !inFile.eof()) {
            throw std::runtime_error("Error reading from binary file. File might be corrupted or format is incorrect.");
        }
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Runtime Error while reading binary file: " << e.what() << std::endl;
        inFile.close();
        return 4;
    }
    inFile.close();

    if (employees.empty()) {
        std::cout << "Reporter: No data found in '" << binaryInputFileName << "' or file is empty." << std::endl;
        // Optionally create an empty report or just exit
    }
    else {
        std::cout << "Reporter: Read " << employees.size() << " records from '" << binaryInputFileName << "'." << std::endl;
    }

    // 3. Sort employees (e.g., by ID)
    std::sort(employees.begin(), employees.end(), compareEmployeesById);

    // 4. Open report file for writing
    // Truncate if exists (overwrite), create if not.
    std::ofstream outFile(reportOutputFileName, std::ios::out | std::ios::trunc);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open report file '" << reportOutputFileName << "' for writing." << std::endl;
        return 3; // Re-using error code 3 for file opening issues
    }

    std::cout << "Reporter: Generating report '" << reportOutputFileName << "'..." << std::endl;

    // 5. Write report
    outFile << "Report for file: \"" << binaryInputFileName << "\"\n";
    outFile << "------------------------------------------------------------------\n";
    outFile << std::left
        << std::setw(10) << "ID"
        << std::setw(MAX_EMPLOYEE_NAME_LENGTH + 6) << "Name" // Adjust width for name
        << std::setw(15) << "Hours Worked"
        << std::setw(15) << "Salary" << std::endl;
    outFile << "------------------------------------------------------------------\n";

    // Set precision for floating point numbers (salary, hours)
    outFile << std::fixed << std::setprecision(2);

    for (const auto& emp : employees) {
        double salary = emp.hoursWorked * hourlyRate;
        outFile << std::left
            << std::setw(10) << emp.id
            << std::setw(MAX_EMPLOYEE_NAME_LENGTH + 6) << emp.name
            << std::setw(15) << emp.hoursWorked
            << std::setw(15) << salary << std::endl;
        if (outFile.fail()) {
            std::cerr << "Error: Failed to write to report file. Disk full or other I/O error?" << std::endl;
            outFile.close(); // Attempt to close
            return 4; // IO error
        }
    }
    outFile << "------------------------------------------------------------------\n";

    outFile.close();
    if (outFile.fail()) { // Check error on close
        std::cerr << "Error: Failed to finalize report file. Disk full or other I/O error?" << std::endl;
        return 4;
    }
    std::cout << "Reporter: Report file '" << reportOutputFileName << "' generated successfully." << std::endl;
    return 0; // Success
}