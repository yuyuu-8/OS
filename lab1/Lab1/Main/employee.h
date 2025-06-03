#pragma once

#include <limits> // Required for std::numeric_limits for input clearing

// Maximum length for employee name (excluding null terminator)
#define MAX_EMPLOYEE_NAME_LENGTH 9 

struct Employee {
    int id;                         // Employee identification number
    char name[MAX_EMPLOYEE_NAME_LENGTH + 1]; // Employee name (+1 for null terminator)
    double hoursWorked;             // Hours worked by the employee
};