#include "Execution.h"

#include "PriorityQueue.h"

#include <chrono>
#include <limits>
#include <vector>

ExecutionResult ExecutionEngine::run(int workerCount) const {
    const auto startClock = std::chrono::steady_clock::now();
    ExecutionResult result;
    result.workerCount = workerCount;

    const int taskCount = graph.size();
    if (workerCount <= 0 || graph.hasCycle()) {
        const auto endClock = std::chrono::steady_clock::now();
        result.calculationTimeMicroseconds =
            std::chrono::duration_cast<std::chrono::microseconds>(endClock - startClock).count();
        return result;
    }

    if (taskCount == 0) {
        result.valid = true;
        const auto endClock = std::chrono::steady_clock::now();
        result.calculationTimeMicroseconds =
            std::chrono::duration_cast<std::chrono::microseconds>(endClock - startClock).count();
        return result;
    }

    std::vector<int> remainingDependencies(taskCount, 0);
    PriorityQueue readyTasks(taskCount);
    for (int id = 0; id < taskCount; ++id) {
        remainingDependencies[id] = static_cast<int>(graph.getPredecessors(id).size());
        if (remainingDependencies[id] == 0) {
            readyTasks.push(id, graph.getTask(id).priority);
        }
    }

    struct Worker {
        bool busy = false;
        int taskId = -1;
        int finishTime = 0;
    };
    std::vector<Worker> workers(workerCount);
    std::vector<bool> completed(taskCount, false);
    int completedCount = 0;
    int currentTime = 0;

    while (completedCount < taskCount) {
        // Finish all work that ends now before starting newly ready tasks.
        for (Worker& worker : workers) {
            if (!worker.busy || worker.finishTime != currentTime) {
                continue;
            }

            completed[worker.taskId] = true;
            ++completedCount;
            worker.busy = false;

            for (int next : graph.getSuccessors(worker.taskId)) {
                --remainingDependencies[next];
                if (remainingDependencies[next] == 0) {
                    readyTasks.push(next, graph.getTask(next).priority);
                }
            }
        }

        for (int workerId = 0; workerId < workerCount && !readyTasks.isEmpty(); ++workerId) {
            if (workers[workerId].busy) {
                continue;
            }

            const int taskId = readyTasks.pop();
            const int finishTime = currentTime + graph.getTask(taskId).duration;
            workers[workerId] = {true, taskId, finishTime};
            result.tasks.push_back({taskId, workerId, currentTime, finishTime});
        }

        if (completedCount == taskCount) {
            break;
        }

        int nextFinish = std::numeric_limits<int>::max();
        for (const Worker& worker : workers) {
            if (worker.busy && worker.finishTime < nextFinish) {
                nextFinish = worker.finishTime;
            }
        }

        if (nextFinish == std::numeric_limits<int>::max()) {
            // No task can run: the dependency graph is not schedulable.
            result.tasks.clear();
            const auto endClock = std::chrono::steady_clock::now();
            result.calculationTimeMicroseconds =
                std::chrono::duration_cast<std::chrono::microseconds>(endClock - startClock).count();
            return result;
        }
        currentTime = nextFinish;
    }

    result.valid = true;
    result.totalTime = currentTime;
    const auto endClock = std::chrono::steady_clock::now();
    result.calculationTimeMicroseconds =
        std::chrono::duration_cast<std::chrono::microseconds>(endClock - startClock).count();
    return result;
}
