#pragma once

#include "Graph.h"

struct BenchmarkResult {
    int repetitions = 0;
    double topologicalSortAverageMicroseconds = 0.0;
    double priorityScheduleAverageMicroseconds = 0.0;
    double criticalPathAverageMicroseconds = 0.0;
    double executionAverageMicroseconds = 0.0;
};

BenchmarkResult benchmarkScheduler(const Graph& graph, int repetitions, int workerCount);
