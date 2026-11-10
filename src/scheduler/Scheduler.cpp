#include "Scheduler.h"
#include "Queue.h"
#include "PriorityQueue.h"

ScheduleResult Scheduler::topologicalSort() const {
    ScheduleResult result;
    int n = graph.size();

    std::vector<int> inDegree(n);
    for (int id = 0; id < n; id++) {
        inDegree[id] = static_cast<int>(graph.getPredecessors(id).size());
    }

    Queue ready(n);
    for (int id = 0; id < n; id++) {
        if (inDegree[id] == 0) ready.enqueue(id);
    }

    while (!ready.isEmpty()) {
        int current = ready.dequeue();
        result.order.push_back(current);

        for (int next : graph.getSuccessors(current)) {
            if (--inDegree[next] == 0) {
                ready.enqueue(next);
            }
        }
    }

    result.valid = (static_cast<int>(result.order.size()) == n);
    return result;
}

ScheduleResult Scheduler::prioritySchedule() const {
    ScheduleResult result;
    int n = graph.size();

    std::vector<int> inDegree(n);
    for (int id = 0; id < n; id++) {
        inDegree[id] = static_cast<int>(graph.getPredecessors(id).size());
    }

    PriorityQueue ready(n);               // your own max-heap
    for (int id = 0; id < n; id++) {
        if (inDegree[id] == 0) ready.push(id, graph.getTask(id).priority);
    }

    while (!ready.isEmpty()) {
        int current = ready.pop();
        result.order.push_back(current);

        for (int next : graph.getSuccessors(current)) {
            if (--inDegree[next] == 0) {
                ready.push(next, graph.getTask(next).priority);
            }
        }
    }

    result.valid = (static_cast<int>(result.order.size()) == n);
    return result;
}