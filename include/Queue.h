#pragma once
#include <stdexcept>

class Queue {
    int* data;
    int capacity;
    int head;
    int count;

public:
    explicit Queue(int cap) : data(new int[cap]), capacity(cap), head(0), count(0) {}
    ~Queue() { delete[] data; }

    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    bool isEmpty() const { return count == 0; }
    bool isFull() const { return count == capacity; }

    bool enqueue(int x) {
        if (isFull()) return false;
        data[(head + count) % capacity] = x;
        count++;
        return true;
    }

    int dequeue() {
        if (isEmpty()) throw std::runtime_error("Queue is empty");
        int x = data[head];
        head = (head + 1) % capacity;
        count--;
        return x;
    }
};