#pragma once
#include <vector>
#include "Task.h"

class Graph {
public:
    bool addTask(const Task& t);
    bool addDependency(int from, int to);
    bool hasCycle() const;

    const Task& getTask(int id) const;
    std::vector<int> getAllTaskIds() const;

    const std::vector<int>& getSuccessors(int id) const;
    const std::vector<int>& getPredecessors(int id) const;

    int size() const;

private:
    std::vector<Task> tasks;
    std::vector<std::vector<int>> successors;
    std::vector<std::vector<int>> predecessors;

    int findIndex(int id) const;
    bool dfs(int index, int state[]) const;
};
