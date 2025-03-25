#include <iostream>
#include <memory>

struct Node {
    int data;
    std::shared_ptr<Node> next;
    Node(int x) : data(x), next(nullptr) {}
};

std::shared_ptr<Node> reverseList(std::shared_ptr<Node> head) {
    std::shared_ptr<Node> prev = nullptr;
    std::shared_ptr<Node> current = head;
    std::shared_ptr<Node> next = nullptr;

    while (current != nullptr) {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }

    return prev;
}

void printList(std::shared_ptr<Node> head) {
    while (head != nullptr) {
        std::cout << head->data << " ";
        head = head->next;
    }
    std::cout << std::endl;
}
