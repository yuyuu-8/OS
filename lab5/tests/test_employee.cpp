#define BOOST_TEST_MODULE EmployeeBehaviorTest
#include <boost/test/included/unit_test.hpp>
#include <fstream>
#include <cstdio>
#include "../include/employee.hpp"

void create_test_file(const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    Employee e1 = { 1, "Anna", 12.5 };
    Employee e2 = { 2, "Ivan", 22.0 };
    Employee e3 = { 3, "Oleg", 30.5 };
    out.write((char*)&e1, sizeof(Employee));
    out.write((char*)&e2, sizeof(Employee));
    out.write((char*)&e3, sizeof(Employee));
}

bool find_employee(const std::string& filename, int id, Employee& result) {
    std::ifstream in(filename, std::ios::binary);
    Employee e;
    while (in.read((char*)&e, sizeof(Employee))) {
        if (e.num == id) {
            result = e;
            return true;
        }
    }
    return false;
}

bool update_employee(const std::string& filename, const Employee& updated) {
    std::fstream file(filename, std::ios::in | std::ios::out | std::ios::binary);
    Employee e;
    while (file.read((char*)&e, sizeof(Employee))) {
        if (e.num == updated.num) {
            file.seekp(-static_cast<std::streamoff>(sizeof(Employee)), std::ios::cur);
            file.write((char*)&updated, sizeof(Employee));
            return true;
        }
    }
    return false;
}

// ------------------- TEST CASES -------------------

BOOST_AUTO_TEST_CASE(ReadExistingEmployee) {
    std::string filename = "test_read.bin";
    create_test_file(filename);

    Employee e;
    bool found = find_employee(filename, 2, e);

    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(e.num, 2);
    BOOST_CHECK_EQUAL(std::string(e.name), "Ivan");
    BOOST_CHECK_CLOSE(e.hours, 22.0, 0.001);

    std::remove(filename.c_str());
}

BOOST_AUTO_TEST_CASE(ReadNonExistentEmployee) {
    std::string filename = "test_read_missing.bin";
    create_test_file(filename);

    Employee e;
    bool found = find_employee(filename, 99, e);

    BOOST_CHECK(!found);

    std::remove(filename.c_str());
}

BOOST_AUTO_TEST_CASE(ModifyEmployee) {
    std::string filename = "test_modify.bin";
    create_test_file(filename);

    Employee updated = { 3, "Olga", 88.8 };
    bool modified = update_employee(filename, updated);
    BOOST_CHECK(modified);

    Employee read_back;
    bool found = find_employee(filename, 3, read_back);
    BOOST_CHECK(found);
    BOOST_CHECK_EQUAL(std::string(read_back.name), "Olga");
    BOOST_CHECK_CLOSE(read_back.hours, 88.8, 0.001);

    std::remove(filename.c_str());
}

BOOST_AUTO_TEST_CASE(ModifyMissingEmployee) {
    std::string filename = "test_missing_modify.bin";
    create_test_file(filename);

    Employee ghost = { 99, "Ghost", 99.9 };
    bool modified = update_employee(filename, ghost);
    BOOST_CHECK(!modified);

    std::remove(filename.c_str());
}
