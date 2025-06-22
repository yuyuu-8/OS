#include <iostream>
#include <string>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <chrono>
#include <vector>

pid_t start_process(const std::string& program, const std::vector<std::string>& args, int& stdin_fd, int& stdout_fd) {
    int stdin_pipe[2];
    int stdout_pipe[2];
    if (pipe(stdin_pipe) == -1 || pipe(stdout_pipe) == -1) {
        perror("pipe");
        exit(1);
    }
    pid_t pid = fork();
    if (pid == 0) {
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        close(stdin_pipe[0]);
        close(stdout_pipe[1]);
        std::vector<char*> c_args;
        c_args.push_back(const_cast<char*>(program.c_str()));
        for (const auto& arg : args) {
            c_args.push_back(const_cast<char*>(arg.c_str()));
        }
        c_args.push_back(NULL);
        execvp(program.c_str(), c_args.data());
        perror("execvp");
        exit(1);
    } else {
        close(stdin_pipe[0]);
        close(stdout_pipe[1]);
        stdin_fd = stdin_pipe[1];
        stdout_fd = stdout_pipe[0];
    }
    return pid;
}

std::string read_output(int fd) {
    char buffer[1024];
    std::string output;
    while (true) {
        ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes <= 0) break;
        buffer[bytes] = '\0';
        output += buffer;
    }
    return output;
}

void test_send_and_read() {
    std::string filename = "queue_test.bin";
    int receiver_stdin, receiver_stdout;
    pid_t receiver_pid = start_process("./receiver", {}, receiver_stdin, receiver_stdout);

    std::string receiver_input = filename + "\n3\n1\n";
    write(receiver_stdin, receiver_input.c_str(), receiver_input.size());
    std::this_thread::sleep_for(std::chrono::seconds(2));

    int sender_stdin, sender_stdout;
    pid_t sender_pid = start_process("./sender", {filename, std::to_string(receiver_pid)}, sender_stdin, sender_stdout);

    std::string sender_input = "send\n123\nquit\n";
    write(sender_stdin, sender_input.c_str(), sender_input.size());

    std::string receiver_command = "read\nquit\n";
    write(receiver_stdin, receiver_command.c_str(), receiver_command.size());

    waitpid(receiver_pid, NULL, 0);
    waitpid(sender_pid, NULL, 0);

    std::string receiver_output = read_output(receiver_stdout);
    if (receiver_output.find("Получено: 123") != std::string::npos) {
        std::cout << "Тест отправки и чтения пройден" << std::endl;
    } else {
        std::cout << "Тест отправки и чтения провален" << std::endl;
    }
}

int main() {
    test_send_and_read();
    return 0;
}