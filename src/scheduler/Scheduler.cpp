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

int Scheduler::estimateCompletion(int numWorkers) const {
    if (numWorkers < 1) return -1;

    int n = graph.size();
    if (n == 0) return 0;

    std::vector<int> inDegree(n);
    for (int id = 0; id < n; id++) {
        inDegree[id] = static_cast<int>(graph.getPredecessors(id).size());
    }

    std::vector<int> finishTime(n, 0);

    PriorityQueue ready(n);      // dependencies done, ordered by task priority
    PriorityQueue running(n);    // executing, ordered by earliest finish time
    for (int id = 0; id < n; id++) {
        if (inDegree[id] == 0) ready.push(id, graph.getTask(id).priority);
    }

    int freeWorkers = numWorkers;
    int time = 0;
    int completed = 0;

    while (!ready.isEmpty() || !running.isEmpty()) {
        // 1. Start as many ready tasks as there are free workers
        while (freeWorkers > 0 && !ready.isEmpty()) {
            int id = ready.pop();
            finishTime[id] = time + graph.getTask(id).duration;
            running.push(id, -finishTime[id]);   // negative: earliest finish pops first
            freeWorkers--;
        }

        // 2. Jump the clock to the earliest finish time
        time = -running.peekPriority();

        // 3. Complete every task finishing at that moment, releasing successors
        while (!running.isEmpty() && -running.peekPriority() == time) {
            int done = running.pop();
            freeWorkers++;
            completed++;

            for (int next : graph.getSuccessors(done)) {
                if (--inDegree[next] == 0) {
                    ready.push(next, graph.getTask(next).priority);
                }
            }
        }
    }

    return (completed == n) ? time : -1;   // fewer than n done means a cycle
}