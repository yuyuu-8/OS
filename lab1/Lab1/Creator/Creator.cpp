#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept> // For std::invalid_argument, std::runtime_error
#include <limits>    // For std::numeric_limits
#include "employee.h" // Assuming it's in the same directory or include paths are set

// Function to read an integer robustly
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

// Function to read a double robustly
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

// Function to read a string (name) robustly
void readEmployeeName(const std::string& prompt, char* buffer, int bufferSize) {
    while (true) {
        std::cout << prompt;
        // std::cin.getline will read up to bufferSize-1 characters and null-terminate
        // It also handles spaces in names.
        std::cin.getline(buffer, bufferSize);

        if (std::cin.good()) { // Successfully read a line (might be empty)
            // Optional: check if name is empty, if that's a requirement
            if (buffer[0] == '\0') {
                // std::cout << "Warning: Name is empty. Proceeding..." << std::endl; // Or treat as error
            }
            return;
        }
        else if (std::cin.fail() && !std::cin.eof()) {
            // This case might happen if the line was too long for the buffer
            // cin.getline sets failbit if it reads bufferSize-1 chars but doesn't find newline
            std::cout << "Warning: Name might have been truncated to " << (bufferSize - 1) << " characters." << std::endl;
            std::cin.clear(); // Clear failbit
            // If failbit was due to too long line, the rest of the line is still in the input buffer.
            // We need to discard it IF getline didn't consume it all (e.g. if it hit EOF before newline)
            // However, typically getline discards the problematic part up to the newline it couldn't store,
            // or until it fills the buffer. If it filled the buffer and stopped, the next char might be the newline.
            // A simple ignore() up to newline is generally safe here after clearing.
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return; // Return with potentially truncated name
        }
        // Handle other unexpected cin errors if necessary, though less common for getline
    }
}


int main(int argc, char* argv[]) {
    // 1. Argument validation
    if (argc != 3) {
        std::cerr << "Usage: Creator.exe <output_binary_file> <num_records>" << std::endl;
        std::cerr << "Error: Incorrect number of arguments." << std::endl;
        return 1; // Error code for incorrect arguments
    }

    const char* outputFileName = argv[1];
    int numRecords;

    try {
        // Use std::stoi for string to int conversion, C++ style
        numRecords = std::stoi(argv[2]);
        if (numRecords <= 0) {
            // Throw an exception for invalid argument value
            throw std::invalid_argument("Number of records must be a positive integer.");
        }
    }
    catch (const std::invalid_argument& e) {
        // Catch exceptions from std::stoi (e.g., "abc") or our custom throw
        std::cerr << "Error: Invalid number of records argument: " << e.what() << std::endl;
        return 2; // Error code for invalid argument value
    }
    catch (const std::out_of_range& e) {
        // Catch exceptions from std::stoi (e.g., number too large for int)
        std::cerr << "Error: Number of records is out of range: " << e.what() << std::endl;
        return 2; // Error code for invalid argument value
    }

    // 2. File opening
    // Open in binary mode, truncate if exists (overwrite), create if not.
    std::ofstream outFile(outputFileName, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!outFile.is_open()) { // Check if file was successfully opened
        std::cerr << "Error: Could not open file '" << outputFileName << "' for writing." << std::endl;
        // Consider GetLastError() here if on Windows and using Windows API file functions,
        // but for fstream, this is usually sufficient.
        // Common reasons: path doesn't exist, permissions issue.
        return 3; // Error code for file opening failure
    }

    std::cout << "Creator: Preparing to write " << numRecords << " records to '" << outputFileName << "'" << std::endl;

    // 3. Data input and writing
    try {
        for (int i = 0; i < numRecords; ++i) {
            Employee emp;
            std::cout << "\n--- Enter data for employee " << i + 1 << " ---" << std::endl;

            emp.id = readInteger("ID (integer): ");

            // Need to consume the newline left by std::cin >> for integer/double
            // before calling std::cin.getline for the name.
            // This is done inside readInteger/readDouble by cin.ignore.
            // If not using helper functions, do it here:
            // std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); 

            readEmployeeName("Name (max " + std::to_string(MAX_EMPLOYEE_NAME_LENGTH) + " chars): ", emp.name, sizeof(emp.name));

            emp.hoursWorked = readDouble("Hours worked (double): ");
            if (emp.hoursWorked < 0) {
                std::cout << "Warning: Negative hours worked entered. Proceeding, but this might be an error." << std::endl;
                // Or treat as an error: throw std::runtime_error("Hours worked cannot be negative.");
            }

            // C++ style cast: reinterpret_cast for writing raw bytes
            outFile.write(reinterpret_cast<const char*>(&emp), sizeof(Employee));

            if (outFile.fail()) { // Check for write errors
                throw std::runtime_error("Failed to write record to file. Disk full or other I/O error?");
            }
            std::cout << "Record " << i + 1 << " written." << std::endl;
        }
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Runtime Error during data input/writing: " << e.what() << std::endl;
        outFile.close(); // Attempt to close file even on error
        return 4; // Error code for runtime/IO error
    }

    outFile.close();
    std::cout << "\nCreator: Binary file '" << outputFileName << "' created successfully with " << numRecords << " records." << std::endl;
    return 0; // Success
}