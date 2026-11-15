#pragma once

#include <vector>

#include "Graph.h"

struct TaskExecution {
    int taskId = -1;
    int workerId = -1;
    int startTime = 0;
    int finishTime = 0;
};

struct ExecutionResult {
    bool valid = false;
    int workerCount = 0;
    int totalTime = 0;
    long long calculationTimeMicroseconds = 0;
    std::vector<TaskExecution> tasks;
};

class ExecutionEngine {
public:
    explicit ExecutionEngine(const Graph& graph) : graph(graph) {}
    ExecutionResult run(int workerCount) const;

private:
    const Graph& graph;
};
