#pragma once
#include <vector>
#include "Graph.h"

struct ScheduleResult {
    std::vector<int> order;   // task IDs in execution order
    bool valid;               // false if a cycle prevented a full ordering
};

struct CriticalPathResult {
    std::vector<int> path;
    int totalDuration;
};

class Scheduler {
public:
    explicit Scheduler(const Graph& g) : graph(g) {}
    ScheduleResult topologicalSort() const;          // Step 2
    ScheduleResult prioritySchedule() const;         // Step 3
    CriticalPathResult criticalPath() const;         // Step 4
    int estimateCompletion(int numWorkers) const;    // Step 5
private:
    const Graph& graph;
};