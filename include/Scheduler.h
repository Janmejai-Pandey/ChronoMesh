#pragma once
#include <vector>
#include "Graph.h"

struct ScheduleResult {
    std::vector<int> order;
    bool valid;
};

struct CriticalPathResult {
    std::vector<int> path;
    int totalDuration;
};

class Scheduler {
public:
    explicit Scheduler(const Graph& g) : graph(g) {}
    ScheduleResult topologicalSort() const;
    ScheduleResult prioritySchedule() const;
    CriticalPathResult criticalPath() const;
    int estimateCompletion(int numWorkers) const;
private:
    const Graph& graph;
};