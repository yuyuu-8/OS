#pragma once

#include <vector>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <string>

// Custom exception classes
class MarkerThreadException : public std::runtime_error {
public:
    explicit MarkerThreadException(const std::string& message)
        : std::runtime_error(message) {}
};

class InvalidInputException : public MarkerThreadException {
public:
    explicit InvalidInputException(const std::string& message)
        : MarkerThreadException(message) {}
};

// Marker thread function
void marker_func(int id,
                std::vector<int>& array,
                std::vector<bool>& terminate_flags,
                std::mutex& mutex,
                std::condition_variable& start_cv,
                std::condition_variable& main_wait_cv,
                std::condition_variable& marker_wait_cv,
                bool& start_signal,
                int& stuck_markers_count);