#include "Benchmark.h"

#include "Execution.h"
#include "Scheduler.h"

#include <chrono>

namespace {
template <typename Function>
double averageTimeMicroseconds(int repetitions, Function function) {
    const auto start = std::chrono::steady_clock::now();
    for (int run = 0; run < repetitions; ++run) {
        function();
    }
    const auto finish = std::chrono::steady_clock::now();
    const double total = std::chrono::duration<double, std::micro>(finish - start).count();
    return total / repetitions;
}
}

BenchmarkResult benchmarkScheduler(const Graph& graph, int repetitions, int workerCount) {
    BenchmarkResult result;
    if (repetitions <= 0 || workerCount <= 0) {
        return result;
    }

    Scheduler scheduler(graph);
    ExecutionEngine engine(graph);
    result.repetitions = repetitions;
    result.topologicalSortAverageMicroseconds = averageTimeMicroseconds(repetitions, [&]() {
        const ScheduleResult value = scheduler.topologicalSort();
        (void)value;
    });
    result.priorityScheduleAverageMicroseconds = averageTimeMicroseconds(repetitions, [&]() {
        const ScheduleResult value = scheduler.prioritySchedule();
        (void)value;
    });
    result.criticalPathAverageMicroseconds = averageTimeMicroseconds(repetitions, [&]() {
        const CriticalPathResult value = scheduler.criticalPath();
        (void)value;
    });
    result.executionAverageMicroseconds = averageTimeMicroseconds(repetitions, [&]() {
        const ExecutionResult value = engine.run(workerCount);
        (void)value;
    });
    return result;
}
