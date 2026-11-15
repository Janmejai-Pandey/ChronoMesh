#pragma once

#include <string>
#include <vector>

#include "Graph.h"

// Life of a task:  Waiting -> Ready -> Running -> Done   (or Failed)
// Blocked = it can never run, because a task it depends on has failed.
enum class TaskState { Waiting, Ready, Running, Done, Failed, Blocked };

// Turns a state into text for printing, e.g. TaskState::Done -> "DONE".
const char* toString(TaskState state);

// One row of the timeline: which worker ran which task, from when to when.
struct TaskExecution {
    int taskId = -1;
    int workerId = -1;
    int startTime = 0;
    int finishTime = 0;
    TaskState status = TaskState::Done;   // Done or Failed once the run is over
};

struct ExecutionResult {
    bool valid = false;
    int workerCount = 0;
    int totalTime = 0;
    long long calculationTimeMicroseconds = 0;
    std::vector<TaskExecution> tasks;   // timeline rows (used by the UI)
    std::vector<TaskState> states;      // final state of every task, index = task id
    std::vector<std::string> log;       // step-by-step execution log
    int doneCount = 0;
    int failedCount = 0;
    int blockedCount = 0;
};

class ExecutionEngine {
public:
    explicit ExecutionEngine(const Graph& graph) : graph(graph) {}

    // failTasks = IDs of tasks that should fail when they finish (empty = nothing fails).
    ExecutionResult run(int workerCount, const std::vector<int>& failTasks = {}) const;

private:
    const Graph& graph;
};

// Prints the execution log and the final state of every task.
void displayExecutionLog(const Graph& graph, const ExecutionResult& execution);
