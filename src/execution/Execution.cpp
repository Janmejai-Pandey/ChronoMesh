#include "Execution.h"

#include "PriorityQueue.h"
#include "Queue.h"

#include <chrono>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

const char* toString(TaskState state) {
    switch (state) {
    case TaskState::Waiting: return "WAITING";
    case TaskState::Ready:   return "READY";
    case TaskState::Running: return "RUNNING";
    case TaskState::Done:    return "DONE";
    case TaskState::Failed:  return "FAILED";
    case TaskState::Blocked: return "BLOCKED";
    }
    return "UNKNOWN";
}

namespace {
long long microsecondsSince(std::chrono::steady_clock::time_point start) {
    const auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now - start).count();
}

std::string at(int time) {
    return "t=" + std::to_string(time) + ": ";
}

// BFS from a failed task: every task that needs it (directly or indirectly)
// can never run, so it becomes Blocked. Returns how many tasks were blocked.
int blockDependents(const Graph& graph, int failedId, int time, ExecutionResult& result) {
    Queue pending(graph.size());
    pending.enqueue(failedId);
    int blocked = 0;

    while (!pending.isEmpty()) {
        const int current = pending.dequeue();
        for (int next : graph.getSuccessors(current)) {
            if (result.states[next] != TaskState::Waiting) {
                continue;   // already blocked earlier
            }
            result.states[next] = TaskState::Blocked;
            ++blocked;
            result.log.push_back(at(time) + graph.getTask(next).name +
                                 " BLOCKED (depends on failed " + graph.getTask(failedId).name + ")");
            pending.enqueue(next);
        }
    }
    return blocked;
}
}

ExecutionResult ExecutionEngine::run(int workerCount, const std::vector<int>& failTasks) const {
    const auto startClock = std::chrono::steady_clock::now();
    ExecutionResult result;
    result.workerCount = workerCount;

    const int taskCount = graph.size();
    if (workerCount <= 0 || graph.hasCycle()) {
        result.calculationTimeMicroseconds = microsecondsSince(startClock);
        return result;
    }

    result.states.assign(taskCount, TaskState::Waiting);
    if (taskCount == 0) {
        result.valid = true;
        result.calculationTimeMicroseconds = microsecondsSince(startClock);
        return result;
    }

    std::vector<bool> willFail(taskCount, false);
    for (int id : failTasks) {
        if (id >= 0 && id < taskCount) {
            willFail[id] = true;
        }
    }

    std::vector<int> remainingDependencies(taskCount, 0);
    PriorityQueue readyTasks(taskCount);
    for (int id = 0; id < taskCount; ++id) {
        remainingDependencies[id] = static_cast<int>(graph.getPredecessors(id).size());
        if (remainingDependencies[id] == 0) {
            readyTasks.push(id, graph.getTask(id).priority);
            result.states[id] = TaskState::Ready;
        }
    }

    struct Worker {
        bool busy = false;
        int taskId = -1;
        int finishTime = 0;
        int row = -1;   // index of this task's row in result.tasks
    };
    std::vector<Worker> workers(workerCount);
    int finishedCount = 0;   // tasks in a final state: Done, Failed or Blocked
    int currentTime = 0;

    while (finishedCount < taskCount) {
        // 1. Finish all work that ends now before starting newly ready tasks.
        for (int workerId = 0; workerId < workerCount; ++workerId) {
            Worker& worker = workers[workerId];
            if (!worker.busy || worker.finishTime != currentTime) {
                continue;
            }

            const int taskId = worker.taskId;
            const std::string& name = graph.getTask(taskId).name;
            worker.busy = false;
            ++finishedCount;

            if (willFail[taskId]) {
                result.states[taskId] = TaskState::Failed;
                result.tasks[worker.row].status = TaskState::Failed;
                ++result.failedCount;
                result.log.push_back(at(currentTime) + "Worker " + std::to_string(workerId) +
                                     " FAILED " + name);
                const int blocked = blockDependents(graph, taskId, currentTime, result);
                result.blockedCount += blocked;
                finishedCount += blocked;
                continue;
            }

            result.states[taskId] = TaskState::Done;
            result.tasks[worker.row].status = TaskState::Done;
            ++result.doneCount;

            std::string unlocked;
            for (int next : graph.getSuccessors(taskId)) {
                --remainingDependencies[next];
                if (remainingDependencies[next] == 0 && result.states[next] == TaskState::Waiting) {
                    readyTasks.push(next, graph.getTask(next).priority);
                    result.states[next] = TaskState::Ready;
                    unlocked += (unlocked.empty() ? "" : ", ") + graph.getTask(next).name;
                }
            }
            result.log.push_back(at(currentTime) + "Worker " + std::to_string(workerId) +
                                 " finished " + name +
                                 (unlocked.empty() ? "" : " -> unlocked " + unlocked));
        }

        // 2. Give ready tasks to free workers (highest priority first).
        for (int workerId = 0; workerId < workerCount && !readyTasks.isEmpty(); ++workerId) {
            if (workers[workerId].busy) {
                continue;
            }

            const int taskId = readyTasks.pop();
            const int finishTime = currentTime + graph.getTask(taskId).duration;
            const int row = static_cast<int>(result.tasks.size());
            workers[workerId] = {true, taskId, finishTime, row};
            result.states[taskId] = TaskState::Running;
            result.tasks.push_back({taskId, workerId, currentTime, finishTime, TaskState::Running});
            result.log.push_back(at(currentTime) + "Worker " + std::to_string(workerId) +
                                 " started " + graph.getTask(taskId).name +
                                 " (until t=" + std::to_string(finishTime) + ")");
        }

        // 3. Every task has reached a final state.
        if (finishedCount == taskCount) {
            break;
        }

        // 4. Jump the clock to the next moment a worker finishes.
        int nextFinish = std::numeric_limits<int>::max();
        for (const Worker& worker : workers) {
            if (worker.busy && worker.finishTime < nextFinish) {
                nextFinish = worker.finishTime;
            }
        }

        if (nextFinish == std::numeric_limits<int>::max()) {
            // No task can run: the dependency graph is not schedulable.
            result.tasks.clear();
            result.log.push_back(at(currentTime) + "stuck - no task can run");
            result.calculationTimeMicroseconds = microsecondsSince(startClock);
            return result;
        }
        currentTime = nextFinish;
    }

    result.valid = true;
    result.totalTime = currentTime;
    result.calculationTimeMicroseconds = microsecondsSince(startClock);
    return result;
}

void displayExecutionLog(const Graph& graph, const ExecutionResult& execution) {
    std::cout << "\n--- Execution Log ---\n";
    if (!execution.valid) {
        std::cout << "No log is available because execution did not complete.\n";
        return;
    }
    for (const std::string& line : execution.log) {
        std::cout << line << '\n';
    }

    std::cout << "\n--- Final Task States ---\n";
    for (int id = 0; id < static_cast<int>(execution.states.size()); ++id) {
        std::cout << '[' << id << ": " << graph.getTask(id).name << "] "
                  << toString(execution.states[id]) << '\n';
    }
    std::cout << "Done: " << execution.doneCount
              << "  Failed: " << execution.failedCount
              << "  Blocked: " << execution.blockedCount << '\n';
}
