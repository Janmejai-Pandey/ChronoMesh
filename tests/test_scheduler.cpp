#include <cassert>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include "Graph.h"
#include "Scheduler.h"
#include "Queue.h"
#include "PriorityQueue.h"

using Edges = std::vector<std::pair<int, int>>;

static Graph build(const std::vector<int>& dur, const std::vector<int>& pri, const Edges& edges) {
    Graph g;
    for (size_t i = 0; i < dur.size(); i++) {
        g.addTask({static_cast<int>(i), "T" + std::to_string(i), dur[i], pri[i]});
    }
    for (const auto& e : edges) g.addDependency(e.first, e.second);
    return g;
}

static void testQueue() {
    Queue q(3);
    assert(q.isEmpty());
    assert(q.enqueue(1) && q.enqueue(2) && q.enqueue(3));
    assert(q.isFull() && !q.enqueue(4));
    assert(q.dequeue() == 1);
    assert(q.enqueue(4));                 // wrap-around
    assert(q.dequeue() == 2 && q.dequeue() == 3 && q.dequeue() == 4);
    assert(q.isEmpty());
}

static void testHeap() {
    PriorityQueue h(4);
    h.push(0, 3);
    h.push(1, 9);
    h.push(2, 9);
    h.push(3, 1);
    assert(h.peekPriority() == 9);
    assert(h.pop() == 1);   // priority 9, lower id first
    assert(h.pop() == 2);
    assert(h.pop() == 0);
    assert(h.pop() == 3);
    assert(h.isEmpty());
}

static void testTopologicalSort() {
    Graph g = build({1, 1, 1, 1}, {1, 1, 1, 1}, {{0, 1}, {0, 2}, {1, 3}, {2, 3}});
    Scheduler s(g);
    ScheduleResult r = s.topologicalSort();
    assert(r.valid);
    assert((r.order == std::vector<int>{0, 1, 2, 3}));

    Graph c = build({1, 1, 1, 1}, {1, 1, 1, 1}, {{0, 1}, {0, 2}, {1, 3}, {2, 3}, {3, 0}});
    assert(!Scheduler(c).topologicalSort().valid);
}

static void testPrioritySchedule() {
    Graph g = build({1, 1, 1, 1}, {1, 5, 9, 7}, {{0, 1}, {0, 2}, {0, 3}});
    ScheduleResult r = Scheduler(g).prioritySchedule();
    assert(r.valid);
    assert((r.order == std::vector<int>{0, 2, 3, 1}));
}

static void testCriticalPath() {
    Graph g = build({2, 3, 5, 1}, {1, 1, 1, 1}, {{0, 1}, {0, 2}, {1, 3}, {2, 3}});
    CriticalPathResult cp = Scheduler(g).criticalPath();
    assert(cp.totalDuration == 8);
    assert((cp.path == std::vector<int>{0, 2, 3}));

    Graph c = build({1, 1}, {1, 1}, {{0, 1}, {1, 0}});
    assert(Scheduler(c).criticalPath().totalDuration == -1);
}

static void testEstimateCompletion() {
    Graph g = build({2, 3, 5, 1}, {1, 1, 1, 1}, {{0, 1}, {0, 2}, {1, 3}, {2, 3}});
    Scheduler s(g);
    assert(s.estimateCompletion(1) == 11);   // sum of durations
    assert(s.estimateCompletion(2) == 8);    // equals critical path
    assert(s.estimateCompletion(10) == 8);   // extra workers don't help
    assert(s.estimateCompletion(0) == -1);

    Graph c = build({1, 1}, {1, 1}, {{0, 1}, {1, 0}});
    assert(Scheduler(c).estimateCompletion(2) == -1);
}

int main() {
    testQueue();
    testHeap();
    testTopologicalSort();
    testPrioritySchedule();
    testCriticalPath();
    testEstimateCompletion();
    std::cout << "All scheduler tests passed.\n";
    return 0;
}
