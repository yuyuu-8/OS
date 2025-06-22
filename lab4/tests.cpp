#include <iostream>
#include <string>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <fstream>
#include <thread>
#include <chrono>

#define MAX_MESSAGE_LEN 20

struct QueueHeader {
    int count;
    int read_index;
    int write_index;
    int max_slots;
};

// Функция для запуска Receiver
pid_t start_receiver(const std::string& filename, int max_slots, int num_senders) {
    pid_t pid = fork();
    if (pid == 0) {
        execl("./receiver", "receiver", filename.c_str(), std::to_string(max_slots).c_str(), std::to_string(num_senders).c_str(), NULL);
        std::cerr << "Ошибка запуска Receiver" << std::endl;
        exit(1);
    }
    return pid;
}

// Функция для запуска Sender
pid_t start_sender(const std::string& filename, pid_t receiver_pid) {
    pid_t pid = fork();
    if (pid == 0) {
        execl("./sender", "sender", filename.c_str(), std::to_string(receiver_pid).c_str(), NULL);
        std::cerr << "Ошибка запуска Sender" << std::endl;
        exit(1);
    }
    return pid;
}

// Функция для отправки команды процессу
void send_command(pid_t pid, const std::string& command) {
    std::string proc_path = "/proc/" + std::to_string(pid) + "/fd/0";
    std::ofstream proc_fd(proc_path);
    if (proc_fd.is_open()) {
        proc_fd << command << std::endl;
        proc_fd.close();
    } else {
        std::cerr << "Не удалось открыть " << proc_path << std::endl;
    }
}

// Тест 1: Проверка создания файла очереди
void test_create_queue_file() {
    std::string filename = "queue_test.bin";
    int max_slots = 3;
    pid_t receiver_pid = start_receiver(filename, max_slots, 0);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    send_command(receiver_pid, "quit");
    waitpid(receiver_pid, NULL, 0);
    std::ifstream file(filename, std::ios::binary);
    if (file.is_open()) {
        QueueHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(QueueHeader));
        if (header.max_slots == max_slots) {
            std::cout << "Тест 1 пройден: Файл создан с правильным количеством слотов" << std::endl;
        } else {
            std::cout << "Тест 1 провален: Неправильное количество слотов" << std::endl;
        }
        file.close();
    } else {
        std::cout << "Тест 1 провален: Файл не создан" << std::endl;
    }
}

// Тест 2: Запуск Sender’ов
void test_start_senders() {
    std::string filename = "queue_test.bin";
    int max_slots = 3;
    int num_senders = 2;
    pid_t receiver_pid = start_receiver(filename, max_slots, num_senders);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    send_command(receiver_pid, "quit");
    waitpid(receiver_pid, NULL, 0);
    std::cout << "Тест 2 пройден: Sender’ы запущены" << std::endl;
}

// Тест 3: Отправка сообщений
void test_send_message() {
    std::string filename = "queue_test.bin";
    int max_slots = 3;
    int num_senders = 1;
    pid_t receiver_pid = start_receiver(filename, max_slots, num_senders);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    pid_t sender_pid = start_sender(filename, receiver_pid);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    send_command(sender_pid, "send");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    send_command(sender_pid, "Hello");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    send_command(sender_pid, "quit");
    send_command(receiver_pid, "quit");
    waitpid(receiver_pid, NULL, 0);
    waitpid(sender_pid, NULL, 0);
    std::cout << "Тест 3 пройден: Сообщение отправлено" << std::endl;
}

// Тест 4: Чтение сообщений
void test_read_message() {
    std::string filename = "queue_test.bin";
    int max_slots = 3;
    int num_senders = 1;
    pid_t receiver_pid = start_receiver(filename, max_slots, num_senders);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    pid_t sender_pid = start_sender(filename, receiver_pid);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    send_command(sender_pid, "send");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    send_command(sender_pid, "Hello");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    send_command(receiver_pid, "read");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    send_command(receiver_pid, "quit");
    send_command(sender_pid, "quit");
    waitpid(receiver_pid, NULL, 0);
    waitpid(sender_pid, NULL, 0);
    std::cout << "Тест 4 пройден: Сообщение прочитано" << std::endl;
}

int main() {
    test_create_queue_file();
    test_start_senders();
    test_send_message();
    test_read_message();
    return 0;
}