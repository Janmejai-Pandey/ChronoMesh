#include "Analytics.h"
#include "Benchmark.h"
#include "Execution.h"
#include "Scheduler.h"
#include "Visualization.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {
bool readInteger(const std::string& prompt, int& value) {
    std::string line;
    while (true) {
        std::cout << prompt;
        if (!std::getline(std::cin, line)) {
            return false;
        }

        std::istringstream input(line);
        char extra = '\0';
        if (input >> value && !(input >> extra)) {
            return true;
        }
        std::cout << "Please enter a whole number.\n";
    }
}

bool readLine(const std::string& prompt, std::string& value) {
    std::cout << prompt;
    return static_cast<bool>(std::getline(std::cin, value));
}

bool wouldCreateCycle(const Graph& graph, int from, int to) {
    if (from < 0 || to < 0 || from >= graph.size() || to >= graph.size()) {
        return false;
    }

    std::vector<bool> visited(graph.size(), false);
    std::vector<int> pending{to};
    while (!pending.empty()) {
        const int current = pending.back();
        pending.pop_back();
        if (current == from) {
            return true;
        }
        if (visited[current]) {
            continue;
        }
        visited[current] = true;
        for (int next : graph.getSuccessors(current)) {
            pending.push_back(next);
        }
    }

    return false;
}

void showMenu() {
    std::cout << "\nChronoMesh - Dependency-Aware Task Scheduler\n"
              << "1. Add task\n"
              << "2. Add dependency\n"
              << "3. Show dependency graph\n"
              << "4. Simulate execution and show analytics\n"
              << "5. Benchmark scheduler\n"
              << "0. Exit\n";
}

void addTask(Graph& graph) {
    std::string name;
    int duration = 0;
    int priority = 0;
    if (!readLine("Task name: ", name) ||
        !readInteger("Duration (positive time units): ", duration) ||
        !readInteger("Priority (higher number runs first): ", priority)) {
        return;
    }

    const int id = graph.size();
    if (graph.addTask({id, name, duration, priority})) {
        std::cout << "Task added with ID " << id << ".\n";
    } else {
        std::cout << "Task was not added. Use a non-empty name and a positive duration.\n";
    }
}

void addDependency(Graph& graph) {
    int from = 0;
    int to = 0;
    if (!readInteger("Task that must finish first (ID): ", from) ||
        !readInteger("Task that waits for it (ID): ", to)) {
        return;
    }

    if (wouldCreateCycle(graph, from, to)) {
        std::cout << "Dependency not added because it would create a cycle.\n";
    } else if (graph.addDependency(from, to)) {
        std::cout << "Dependency added.\n";
    } else {
        std::cout << "Dependency not added. Check the IDs and duplicates.\n";
    }
}

void simulate(Graph& graph) {
    int workerCount = 0;
    if (!readInteger("Number of workers: ", workerCount)) {
        return;
    }

    const ExecutionResult execution = ExecutionEngine(graph).run(workerCount);
    displayWorkerTimeline(graph, execution);
    const CriticalPathResult criticalPath = Scheduler(graph).criticalPath();
    displayAnalytics(graph, execution, criticalPath);
}

void benchmark(const Graph& graph) {
    int workerCount = 0;
    if (!readInteger("Number of workers: ", workerCount)) {
        return;
    }

    const int repetitions = 100;
    const BenchmarkResult result = benchmarkScheduler(graph, repetitions, workerCount);
    if (result.repetitions == 0) {
        std::cout << "Enter a positive worker count to run the benchmark.\n";
        return;
    }

    std::cout << "\nAverage calculation time over " << result.repetitions << " runs (microseconds):\n"
              << "Topological sort: " << result.topologicalSortAverageMicroseconds << '\n'
              << "Priority schedule: " << result.priorityScheduleAverageMicroseconds << '\n'
              << "Critical path: " << result.criticalPathAverageMicroseconds << '\n'
              << "Parallel simulation: " << result.executionAverageMicroseconds << '\n';
}
}

int main() {
    Graph graph;
    int choice = -1;

    while (true) {
        showMenu();
        if (!readInteger("Choice: ", choice)) {
            break;
        }

        switch (choice) {
        case 1:
            addTask(graph);
            break;
        case 2:
            addDependency(graph);
            break;
        case 3:
            displayDependencyGraph(graph);
            break;
        case 4:
            simulate(graph);
            break;
        case 5:
            benchmark(graph);
            break;
        case 0:
            std::cout << "Goodbye.\n";
            return 0;
        default:
            std::cout << "Unknown menu option.\n";
        }
    }

    return 0;
}
