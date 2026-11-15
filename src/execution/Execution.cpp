#include "Execution.h"

#include "PriorityQueue.h"
#include "Queue.h"

#include <chrono>
#include <limits>
#include <string>
#include <vector>

namespace {
// Life of a task:  Waiting -> Ready -> Running -> Done   (or Failed)
// Blocked = it can never run, because a task it depends on has failed.
enum class TaskState { Waiting, Ready, Running, Done, Failed, Blocked };

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

long long microsecondsSince(std::chrono::steady_clock::time_point start) {
    const auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now - start).count();
}

std::string at(int time) {
    return "t=" + std::to_string(time) + ": ";
}

// BFS from a failed task: every task that needs it (directly or indirectly)
// can never run, so it becomes Blocked.
void blockDependents(const Graph& graph, int failedId, int time,
                     std::vector<TaskState>& states, int& finishedCount,
                     std::vector<std::string>* log) {
    Queue pending(graph.size());
    pending.enqueue(failedId);

    while (!pending.isEmpty()) {
        const int current = pending.dequeue();
        for (int next : graph.getSuccessors(current)) {
            if (states[next] != TaskState::Waiting) {
                continue;   // already blocked earlier
            }
            states[next] = TaskState::Blocked;
            ++finishedCount;
            if (log != nullptr) {
                log->push_back(at(time) + graph.getTask(next).name +
                               " BLOCKED (depends on failed " + graph.getTask(failedId).name + ")");
            }
            pending.enqueue(next);
        }
    }
}

// The whole simulation. log == nullptr means "do not write a log" (keeps run() fast for benchmarks).
ExecutionResult simulate(const Graph& graph, int workerCount, const std::vector<int>& failTasks,
                         std::vector<TaskState>& states, std::vector<std::string>* log) {
    const auto startClock = std::chrono::steady_clock::now();
    ExecutionResult result;
    result.workerCount = workerCount;

    const int taskCount = graph.size();
    if (workerCount <= 0 || graph.hasCycle()) {
        result.calculationTimeMicroseconds = microsecondsSince(startClock);
        return result;
    }

    states.assign(taskCount, TaskState::Waiting);
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
            states[id] = TaskState::Ready;
        }
    }

    struct Worker {
        bool busy = false;
        int taskId = -1;
        int finishTime = 0;
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
            worker.busy = false;
            ++finishedCount;

            if (willFail[taskId]) {
                states[taskId] = TaskState::Failed;
                if (log != nullptr) {
                    log->push_back(at(currentTime) + "Worker " + std::to_string(workerId) +
                                   " FAILED " + graph.getTask(taskId).name);
                }
                blockDependents(graph, taskId, currentTime, states, finishedCount, log);
                continue;
            }

            states[taskId] = TaskState::Done;
            std::string unlocked;
            for (int next : graph.getSuccessors(taskId)) {
                --remainingDependencies[next];
                if (remainingDependencies[next] == 0 && states[next] == TaskState::Waiting) {
                    readyTasks.push(next, graph.getTask(next).priority);
                    states[next] = TaskState::Ready;
                    if (log != nullptr) {
                        unlocked += (unlocked.empty() ? "" : ", ") + graph.getTask(next).name;
                    }
                }
            }
            if (log != nullptr) {
                log->push_back(at(currentTime) + "Worker " + std::to_string(workerId) +
                               " finished " + graph.getTask(taskId).name +
                               (unlocked.empty() ? "" : " -> unlocked " + unlocked));
            }
        }

        // 2. Give ready tasks to free workers (highest priority first).
        for (int workerId = 0; workerId < workerCount && !readyTasks.isEmpty(); ++workerId) {
            if (workers[workerId].busy) {
                continue;
            }

            const int taskId = readyTasks.pop();
            const int finishTime = currentTime + graph.getTask(taskId).duration;
            workers[workerId] = {true, taskId, finishTime};
            states[taskId] = TaskState::Running;
            result.tasks.push_back({taskId, workerId, currentTime, finishTime});
            if (log != nullptr) {
                log->push_back(at(currentTime) + "Worker " + std::to_string(workerId) +
                               " started " + graph.getTask(taskId).name +
                               " (until t=" + std::to_string(finishTime) + ")");
            }
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
            if (log != nullptr) {
                log->push_back(at(currentTime) + "stuck - no task can run");
            }
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
}

// Same as before for the whole team: no failures, no log.
ExecutionResult ExecutionEngine::run(int workerCount) const {
    std::vector<TaskState> states;
    return simulate(graph, workerCount, {}, states, nullptr);
}

// Person 3 extra: run with failing tasks, and also return
// the final state of every task ("DONE", "FAILED", "BLOCKED") and a step-by-step log.
// Not in Execution.h yet (headers are kept unchanged) - add this declaration
// to Execution.h when the team updates the header.
ExecutionResult runWithFailures(const Graph& graph, int workerCount,
                                const std::vector<int>& failTasks,
                                std::vector<std::string>& states,
                                std::vector<std::string>& log) {
    std::vector<TaskState> finalStates;
    log.clear();
    const ExecutionResult result = simulate(graph, workerCount, failTasks, finalStates, &log);

    states.clear();
    for (TaskState state : finalStates) {
        states.push_back(toString(state));
    }
    return result;
}
