#include "Analytics.h"

#include <iomanip>
#include <iostream>

void displayAnalytics(const Graph& graph,
                      const ExecutionResult& execution,
                      const CriticalPathResult& criticalPath) {
    std::cout << "\n--- Execution Statistics ---\n";
    std::cout << "Tasks: " << graph.size() << '\n';

    int dependencyCount = 0;
    int totalWork = 0;
    for (int id = 0; id < graph.size(); ++id) {
        dependencyCount += static_cast<int>(graph.getSuccessors(id).size());
        totalWork += graph.getTask(id).duration;
    }
    std::cout << "Dependencies: " << dependencyCount << '\n';

    if (!execution.valid) {
        std::cout << "Execution could not be completed. Check the worker count and dependencies.\n";
        return;
    }

    std::cout << "Workers: " << execution.workerCount << '\n';
    std::cout << "Simulated completion time: " << execution.totalTime << " time units\n";
    std::cout << "Total task work: " << totalWork << " time units\n";
    std::cout << "Scheduler calculation time: " << execution.calculationTimeMicroseconds << " microseconds\n";

    const double throughput = execution.totalTime == 0
        ? 0.0
        : static_cast<double>(graph.size()) / execution.totalTime;
    const double utilization = execution.totalTime == 0 || execution.workerCount == 0
        ? 0.0
        : 100.0 * totalWork / (execution.workerCount * execution.totalTime);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Throughput: " << throughput << " tasks per time unit\n";
    std::cout << "Worker utilization: " << utilization << "%\n";

    if (criticalPath.totalDuration >= 0) {
        std::cout << "Critical path duration: " << criticalPath.totalDuration << " time units\n";
        std::cout << "Critical path: ";
        for (std::size_t index = 0; index < criticalPath.path.size(); ++index) {
            const int id = criticalPath.path[index];
            if (index != 0) {
                std::cout << " -> ";
            }
            std::cout << id << ':' << graph.getTask(id).name;
        }
        std::cout << '\n';
    }
}
