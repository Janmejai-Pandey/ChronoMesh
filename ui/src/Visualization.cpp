#include "Visualization.h"

#include "Scheduler.h"

#include <algorithm>
#include <iostream>
#include <vector>

void displayDependencyGraph(const Graph& graph) {
    std::cout << "\n--- Dependency Graph ---\n";
    if (graph.size() == 0) {
        std::cout << "No tasks have been added.\n";
        return;
    }

    Scheduler scheduler(graph);
    const ScheduleResult order = scheduler.topologicalSort();
    if (!order.valid) {
        std::cout << "The dependency graph contains a cycle.\n";
        return;
    }

    std::vector<int> level(graph.size(), 0);
    int largestLevel = 0;
    for (int id : order.order) {
        for (int next : graph.getSuccessors(id)) {
            level[next] = std::max(level[next], level[id] + 1);
            largestLevel = std::max(largestLevel, level[next]);
        }
    }

    std::cout << "Stages show tasks that can become ready at the same dependency depth.\n";
    for (int currentLevel = 0; currentLevel <= largestLevel; ++currentLevel) {
        std::cout << "Stage " << currentLevel << ": ";
        bool printed = false;
        for (int id = 0; id < graph.size(); ++id) {
            if (level[id] != currentLevel) {
                continue;
            }
            if (printed) {
                std::cout << "   ";
            }
            std::cout << '[' << id << ": " << graph.getTask(id).name << ']';
            printed = true;
        }
        std::cout << '\n';
    }

    std::cout << "Dependencies:\n";
    bool hasEdges = false;
    for (int id = 0; id < graph.size(); ++id) {
        for (int next : graph.getSuccessors(id)) {
            std::cout << "  [" << id << ": " << graph.getTask(id).name << "] -> ["
                      << next << ": " << graph.getTask(next).name << "]\n";
            hasEdges = true;
        }
    }
    if (!hasEdges) {
        std::cout << "  No dependencies.\n";
    }
}

void displayWorkerTimeline(const Graph& graph, const ExecutionResult& execution) {
    std::cout << "\n--- Worker Timelines ---\n";
    if (!execution.valid) {
        std::cout << "No timeline is available because execution did not complete.\n";
        return;
    }

    for (int workerId = 0; workerId < execution.workerCount; ++workerId) {
        std::vector<TaskExecution> workerTasks;
        for (const TaskExecution& task : execution.tasks) {
            if (task.workerId == workerId) {
                workerTasks.push_back(task);
            }
        }
        std::sort(workerTasks.begin(), workerTasks.end(), [](const TaskExecution& left, const TaskExecution& right) {
            return left.startTime < right.startTime;
        });

        std::cout << "Worker " << workerId << ": ";
        if (workerTasks.empty()) {
            std::cout << "idle\n";
            continue;
        }
        for (std::size_t index = 0; index < workerTasks.size(); ++index) {
            if (index != 0) {
                std::cout << "  ";
            }
            const TaskExecution& task = workerTasks[index];
            std::cout << '[' << task.startTime << '-' << task.finishTime << ": "
                      << graph.getTask(task.taskId).name << ']';
        }
        std::cout << '\n';
    }
    std::cout << "Time range: 0 to " << execution.totalTime << " time units.\n";
}
