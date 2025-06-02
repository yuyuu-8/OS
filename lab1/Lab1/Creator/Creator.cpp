#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "employee.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: Creator.exe <output_binary_file> <num_records>\n";
        return 1;
    }

    const char* fileName = argv[1];
    int numRecords = std::atoi(argv[2]);

    if (numRecords <= 0) {
        std::cerr << "Number of records must be positive.\n";
        return 1;
    }

    std::ofstream outFile(fileName, std::ios::binary | std::ios::out);
    if (!outFile) {
        std::cerr << "Error opening file " << fileName << " for writing.\n";
        return 1;
    }

    std::cout << "Creator: Preparing to write " << numRecords << " records to " << fileName << std::endl;

    for (int i = 0; i < numRecords; ++i) {
        employee emp;
        std::cout << "Enter data for employee " << i + 1 << ":" << std::endl;

        std::cout << "ID (int): ";
        std::cin >> emp.num;

        std::cout << "Name (max 9 chars): ";
        std::cin.ignore(); // съесть newline после числа
        std::cin.width(10); // ограничение для std::cin >> char[]
        std::cin >> emp.name;

        std::cout << "Hours (double): ";
        std::cin >> emp.hours;

        outFile.write(reinterpret_cast<const char*>(&emp), sizeof(employee));
        std::cout << "Record " << i + 1 << " written." << std::endl;
    }

    outFile.close();
    std::cout << "Creator: Binary file " << fileName << " created successfully.\n";
    return 0;
}