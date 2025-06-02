#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm> // ��� std::sort
#include <iomanip>   // ��� std::setw, std::fixed, std::setprecision
#include "employee.h"

// ������� ��� ��������� ��� ����������
bool compareEmployees(const employee& a, const employee& b) {
    return a.num < b.num;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: Reporter.exe <input_binary_file> <output_report_file> <hourly_rate>\n";
        return 1;
    }

    const char* binaryFileName = argv[1];
    const char* reportFileName = argv[2];
    double hourlyRate = std::atof(argv[3]); // atof ���� �����������

    if (hourlyRate <= 0) {
        std::cerr << "Hourly rate must be positive.\n";
        return 1;
    }

    std::ifstream inFile(binaryFileName, std::ios::binary | std::ios::in);
    if (!inFile) {
        std::cerr << "Error opening binary file " << binaryFileName << " for reading.\n";
        return 1;
    }

    std::vector<employee> employees;
    employee emp;
    while (inFile.read(reinterpret_cast<char*>(&emp), sizeof(employee))) {
        employees.push_back(emp);
    }
    inFile.close();

    if (employees.empty()) {
        std::cout << "Reporter: No data found in " << binaryFileName << std::endl;
    }
    else {
        std::cout << "Reporter: Read " << employees.size() << " records from " << binaryFileName << std::endl;
    }


    std::sort(employees.begin(), employees.end(), compareEmployees);

    std::ofstream outFile(reportFileName);
    if (!outFile) {
        std::cerr << "Error opening report file " << reportFileName << " for writing.\n";
        return 1;
    }

    std::cout << "Reporter: Generating report " << reportFileName << std::endl;

    outFile << "����� �� ����� \"" << binaryFileName << "\".\n";
    outFile << "------------------------------------------------------------------\n";
    outFile << std::left // ������������ �� ������ ����
        << std::setw(10) << "ID"
        << std::setw(15) << "���"
        << std::setw(10) << "����"
        << std::setw(15) << "��������" << std::endl;
    outFile << "------------------------------------------------------------------\n";

    outFile << std::fixed << std::setprecision(2); // ��� �����

    for (const auto& e : employees) {
        double salary = e.hours * hourlyRate;
        outFile << std::left
            << std::setw(10) << e.num
            << std::setw(15) << e.name
            << std::setw(10) << e.hours
            << std::setw(15) << salary << std::endl;
    }
    outFile << "------------------------------------------------------------------\n";

    outFile.close();
    std::cout << "Reporter: Report file " << reportFileName << " generated successfully.\n";
    return 0;
}