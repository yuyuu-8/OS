#include <iostream>
#include <string>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

#define MAX_MESSAGE_LEN 20

struct QueueHeader {
    int count;
    int read_index;
    int write_index;
    int max_slots;
};

void send_message(const std::string& filename, const std::string& message) {
    FILE* fp = fopen(filename.c_str(), "rb+");
    if (!fp) {
        std::cerr << "Ошибка открытия файла" << std::endl;
        return;
    }
    int fd = fileno(fp);
    while (true) {
        if (flock(fd, LOCK_EX) == -1) {
            std::cerr << "Ошибка блокировки файла" << std::endl;
            fclose(fp);
            return;
        }
        QueueHeader header;
        fseek(fp, 0, SEEK_SET);
        fread(&header, sizeof(QueueHeader), 1, fp);
        if (header.count < header.max_slots) {
            char msg_buf[MAX_MESSAGE_LEN] = {0};
            strncpy(msg_buf, message.c_str(), MAX_MESSAGE_LEN);
            fseek(fp, sizeof(QueueHeader) + header.write_index * MAX_MESSAGE_LEN, SEEK_SET);
            fwrite(msg_buf, MAX_MESSAGE_LEN, 1, fp);
            header.write_index = (header.write_index + 1) % header.max_slots;
            header.count++;
            fseek(fp, 0, SEEK_SET);
            fwrite(&header, sizeof(QueueHeader), 1, fp);
            flock(fd, LOCK_UN);
            std::cout << "Сообщение отправлено" << std::endl;
            break;
        } else {
            flock(fd, LOCK_UN);
            std::cout << "Очередь полна, ждем..." << std::endl;
            sleep(1);
        }
    }
    fclose(fp);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Использование: " << argv[0] << " <имя_файла> <pid_приемника>" << std::endl;
        return 1;
    }
    std::string filename = argv[1];
    pid_t receiver_pid = std::stoi(argv[2]);
    kill(receiver_pid, SIGUSR1);
    while (true) {
        std::string command;
        std::cout << "Введите команду (send/quit): ";
        std::cin >> command;
        if (command == "send") {
            std::string message;
            std::cout << "Введите сообщение: ";
            std::cin >> message;
            if (message.length() > MAX_MESSAGE_LEN) {
                message = message.substr(0, MAX_MESSAGE_LEN);
            }
            send_message(filename, message);
        } else if (command == "quit") {
            break;
        }
    }
    return 0;
}