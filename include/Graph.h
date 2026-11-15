#pragma once
#include <vector>
#include "Task.h"

class Graph {
public:
    bool addTask(const Task& t);
    bool addDependency(int from, int to);   // 'from' must finish before 'to'
    bool hasCycle() const;                  // Person 1's validation

    const Task& getTask(int id) const;
    std::vector<int> getAllTaskIds() const;
    const std::vector<int>& getSuccessors(int id) const;     // tasks that depend on id
    const std::vector<int>& getPredecessors(int id) const;   // tasks id depends on
    int size() const;
private:
    // Zero-based task IDs are used as indices by the scheduler.
    std::vector<Task> tasks;
    std::vector<std::vector<int>> successors;
    std::vector<std::vector<int>> predecessors;
};
