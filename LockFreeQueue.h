#ifndef __LOCKFREEQUEUE_H__
#define __LOCKFREEQUEUE_H__

#include <atomic>
#include <cassert>

template <typename T>
class LockFreeQueue {
private:
    struct Node {
        Node *next;
        T data;

        Node(const T &value) : next(nullptr), data(value) {}
    };

    std::atomic<Node *> head;
    std::atomic<Node *> tail;

public:
    LockFreeQueue() {
        Node *initialNode = new Node(T()); // 初始化一个占位的节点
        head = tail = initialNode;
    }

    ~LockFreeQueue() {
        while (dequeue()) {
            // 在析构时清空队列
        }
    }

    void enqueue(const T &value) {
        Node *newNode = new Node(value);
        Node *tailNode = tail.exchange(newNode);             // CAS设置尾节点
        tailNode->next = newNode;                            // 设置新节点为下一个
        std::atomic_thread_fence(std::memory_order_release); // 发布新节点
    }

    bool dequeue(T &value) {
        Node *headNode = head.load(std::memory_order_acquire); // 获取头节点
        if (headNode == tail.load()) {
            return false; // 队列为空
        }

        Node *newHead = headNode->next;
        value = newHead->data;
        delete headNode;
        head = newHead;
        return true;
    }
};

#endif