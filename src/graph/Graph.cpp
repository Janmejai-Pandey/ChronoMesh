#include "Graph.h"

#include <stdexcept>
#include <vector>

bool Graph::addTask(const Task& task) {
    const int expectedId = static_cast<int>(tasks.size());
    if (task.id != expectedId || task.name.empty() || task.duration <= 0) {
        return false;
    }

    tasks.push_back(task);
    successors.emplace_back();
    predecessors.emplace_back();
    return true;
}

bool Graph::addDependency(int from, int to) {
    if (from < 0 || to < 0 || from >= size() || to >= size() || from == to) {
        return false;
    }

    for (int next : successors[from]) {
        if (next == to) {
            return false;
        }
    }

    // A new edge from -> to would make a cycle if to can already reach from.
    std::vector<bool> visited(tasks.size(), false);
    std::vector<int> pending{to};
    while (!pending.empty()) {
        const int current = pending.back();
        pending.pop_back();
        if (current == from) {
            return false;
        }
        if (visited[current]) {
            continue;
        }
        visited[current] = true;
        for (int next : successors[current]) {
            pending.push_back(next);
        }
    }

    successors[from].push_back(to);
    predecessors[to].push_back(from);
    return true;
}

bool Graph::hasCycle() const {
    std::vector<int> state(tasks.size(), 0);

    for (int start = 0; start < size(); ++start) {
        if (state[start] != 0) {
            continue;
        }

        std::vector<int> nodes{start};
        std::vector<std::size_t> nextChild{0};
        state[start] = 1;

        while (!nodes.empty()) {
            const int current = nodes.back();
            std::size_t& childIndex = nextChild.back();

            if (childIndex == successors[current].size()) {
                state[current] = 2;
                nodes.pop_back();
                nextChild.pop_back();
                continue;
            }

            const int child = successors[current][childIndex++];
            if (state[child] == 1) {
                return true;
            }
            if (state[child] == 0) {
                state[child] = 1;
                nodes.push_back(child);
                nextChild.push_back(0);
            }
        }
    }

    return false;
}

const Task& Graph::getTask(int id) const {
    if (id < 0 || id >= size()) {
        throw std::out_of_range("Task ID does not exist");
    }
    return tasks[id];
}

std::vector<int> Graph::getAllTaskIds() const {
    std::vector<int> ids;
    for (int id = 0; id < size(); ++id) {
        ids.push_back(id);
    }
    return ids;
}

const std::vector<int>& Graph::getSuccessors(int id) const {
    if (id < 0 || id >= size()) {
        throw std::out_of_range("Task ID does not exist");
    }
    return successors[id];
}

const std::vector<int>& Graph::getPredecessors(int id) const {
    if (id < 0 || id >= size()) {
        throw std::out_of_range("Task ID does not exist");
    }
    return predecessors[id];
}

int Graph::size() const {
    return static_cast<int>(tasks.size());
}
