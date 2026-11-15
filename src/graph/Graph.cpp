#include <iostream>
#include <stdexcept>
#include "Graph.h"

using namespace std;

// Add a new task
bool Graph::addTask(const Task& t) {
    if (t.id != (int)tasks.size())
        return false;

    if (t.name.empty() || t.duration <= 0 || t.priority < 0)
        return false;

    tasks.push_back(t);
    successors.push_back(vector<int>());
    predecessors.push_back(vector<int>());

    return true;
}

// Add dependency
bool Graph::addDependency(int from, int to) {
    int n = (int)tasks.size();

    if (from < 0 || to < 0 || from >= n || to >= n || from == to)
        return false;

    // Check duplicate dependency
    for (int i = 0; i < (int)successors[from].size(); i++) {
        if (successors[from][i] == to)
            return false;
    }

    // Add dependency
    successors[from].push_back(to);
    predecessors[to].push_back(from);

    // Reject dependency if it creates a cycle
    if (hasCycle()) {
        successors[from].pop_back();
        predecessors[to].pop_back();
        return false;
    }

    return true;
}

// DFS cycle detection
bool Graph::hasCycle() const {
    int n = (int)tasks.size();
    vector<int> state(n, 0);

    auto dfs = [&](auto&& self, int u) -> bool {
        state[u] = 1;

        for (int i = 0; i < (int)successors[u].size(); i++) {
            int v = successors[u][i];

            if (v < 0 || v >= n)
                continue;

            if (state[v] == 1)
                return true;

            if (state[v] == 0 && self(self, v))
                return true;
        }

        state[u] = 2;
        return false;
    };

    for (int i = 0; i < n; i++) {
        if (state[i] == 0 && dfs(dfs, i))
            return true;
    }

    return false;
}

// Get task by ID
const Task& Graph::getTask(int id) const {
    if (id < 0 || id >= (int)tasks.size())
        throw out_of_range("Task ID not found");

    return tasks[id];
}

// Get all task IDs
vector<int> Graph::getAllTaskIds() const {
    vector<int> ids;

    for (int i = 0; i < (int)tasks.size(); i++)
        ids.push_back(tasks[i].id);

    return ids;
}

// Get successor tasks
const vector<int>& Graph::getSuccessors(int id) const {
    if (id < 0 || id >= (int)successors.size())
        throw out_of_range("Task ID not found");

    return successors[id];
}

// Get predecessor tasks
const vector<int>& Graph::getPredecessors(int id) const {
    if (id < 0 || id >= (int)predecessors.size())
        throw out_of_range("Task ID not found");

    return predecessors[id];
}

// Get total number of tasks
int Graph::size() const {
    return (int)tasks.size();
}
