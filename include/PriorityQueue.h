#pragma once

class PriorityQueue {
    struct Item {
        int priority;
        int id;
    };

    Item* heap;
    int capacity;
    int size;

    // true if a should leave the heap before b
    static bool higher(const Item& a, const Item& b) {
        if (a.priority != b.priority) return a.priority > b.priority;
        return a.id < b.id;
    }

    void swapItems(int i, int j) {
        Item t = heap[i];
        heap[i] = heap[j];
        heap[j] = t;
    }

    void siftUp(int i) {
        while (i > 0) {
            int parent = (i - 1) / 2;
            if (!higher(heap[i], heap[parent])) break;
            swapItems(i, parent);
            i = parent;
        }
    }

    void siftDown(int i) {
        while (true) {
            int left = 2 * i + 1;
            int right = 2 * i + 2;
            int best = i;
            if (left < size && higher(heap[left], heap[best])) best = left;
            if (right < size && higher(heap[right], heap[best])) best = right;
            if (best == i) break;
            swapItems(i, best);
            i = best;
        }
    }

public:
    explicit PriorityQueue(int cap) : heap(new Item[cap]), capacity(cap), size(0) {}
    ~PriorityQueue() { delete[] heap; }

    PriorityQueue(const PriorityQueue&) = delete;
    PriorityQueue& operator=(const PriorityQueue&) = delete;

    bool isEmpty() const { return size == 0; }
    bool isFull() const { return size == capacity; }

    bool push(int id, int priority) {
        if (isFull()) return false;
        heap[size] = {priority, id};
        siftUp(size);
        size++;
        return true;
    }

    int pop() {
        int topId = heap[0].id;
        size--;
        heap[0] = heap[size];
        siftDown(0);
        return topId;
    }

        int peekPriority() const {
        return heap[0].priority;
    }
};