#include "Scheduler.h"
#include "Queue.h"
#include "PriorityQueue.h"

#include <algorithm>

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

    PriorityQueue ready(n);
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

CriticalPathResult Scheduler::criticalPath() const {
    CriticalPathResult result;
    result.totalDuration = -1;

    ScheduleResult topo = topologicalSort();
    if (!topo.valid) return result;

    int n = graph.size();
    if (n == 0) {
        result.totalDuration = 0;
        return result;
    }

    std::vector<int> finish(n, 0);
    std::vector<int> prev(n, -1);

    for (int id : topo.order) {
        int start = 0;
        for (int p : graph.getPredecessors(id)) {
            if (finish[p] > start) {
                start = finish[p];
                prev[id] = p;
            }
        }
        finish[id] = start + graph.getTask(id).duration;
    }


    int end = 0;
    for (int id = 1; id < n; id++) {
        if (finish[id] > finish[end]) end = id;
    }
    result.totalDuration = finish[end];


    for (int cur = end; cur != -1; cur = prev[cur]) {
        result.path.push_back(cur);
    }
    std::reverse(result.path.begin(), result.path.end());

    return result;
}