#include "Benchmark.h"
#include "Execution.h"
#include "Scheduler.h"

#include <cassert>
#include <iostream>

namespace {
Graph makeSampleGraph() {
    Graph graph;
    assert(graph.addTask({0, "Prepare", 3, 1}));
    assert(graph.addTask({1, "Download", 2, 5}));
    assert(graph.addTask({2, "Build", 4, 3}));
    assert(graph.addDependency(0, 2));
    assert(graph.addDependency(1, 2));
    return graph;
}

void testGraphAndScheduler() {
    Graph graph = makeSampleGraph();
    assert(graph.size() == 3);
    assert(graph.getAllTaskIds().size() == 3);
    assert(!graph.hasCycle());

    const ScheduleResult topological = Scheduler(graph).topologicalSort();
    assert(topological.valid);
    assert(topological.order.size() == 3);
    assert(topological.order[2] == 2);

    const ScheduleResult priority = Scheduler(graph).prioritySchedule();
    assert(priority.valid);
    assert(priority.order[0] == 1);

    const CriticalPathResult path = Scheduler(graph).criticalPath();
    assert(path.totalDuration == 7);
    assert(path.path.size() == 2);
    assert(path.path[0] == 0 && path.path[1] == 2);
}

void testValidationAndExecution() {
    Graph graph = makeSampleGraph();
    assert(graph.addDependency(2, 0));
    assert(graph.hasCycle());
    assert(!Scheduler(graph).topologicalSort().valid);
    assert(!ExecutionEngine(graph).run(2).valid);
}

void testExecutionAndInputValidation() {
    Graph graph = makeSampleGraph();
    assert(!graph.addDependency(0, 2));
    assert(!graph.addTask({3, "", 1, 0}));
    assert(!graph.addTask({3, "Invalid duration", 0, 0}));

    const ExecutionResult result = ExecutionEngine(graph).run(2);
    assert(result.valid);
    assert(result.totalTime == 7);
    assert(result.tasks.size() == 3);
    assert(result.tasks[0].taskId == 1);
    assert(result.tasks[0].workerId == 0);
    assert(Scheduler(graph).estimateCompletion(2) == 7);
    assert(!ExecutionEngine(graph).run(0).valid);
}

void testBenchmark() {
    Graph graph = makeSampleGraph();
    const BenchmarkResult result = benchmarkScheduler(graph, 5, 2);
    assert(result.repetitions == 5);
    assert(result.topologicalSortAverageMicroseconds >= 0.0);
    assert(result.priorityScheduleAverageMicroseconds >= 0.0);
    assert(result.criticalPathAverageMicroseconds >= 0.0);
    assert(result.executionAverageMicroseconds >= 0.0);
    assert(benchmarkScheduler(graph, 0, 2).repetitions == 0);
}
}

int main() {
    testGraphAndScheduler();
    testValidationAndExecution();
    testExecutionAndInputValidation();
    testBenchmark();
    std::cout << "ChronoMesh tests passed.\n";
    return 0;
}
