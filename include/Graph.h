#pragma once
#include "Task.h"

struct Node {
    int id;
    Node* next;
};

class Graph {
public:
    Graph();
    ~Graph();

    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;

    int addTask(const char* nm);
    bool removeTask(int id);
    void displayTasks() const;

    bool addDependency(int a, int b);
    bool removeDependency(int a, int b);
    void displayGraph() const;

    bool wouldCreateCycle(int a, int b) const;
    bool hasCycle() const;
    bool validateGraph() const;

    bool isValidTask(int id) const;
    bool hasDependency(int a, int b) const;
    const char* getTaskName(int id) const;
    const Node* getDependents(int id) const;
    int getInDegree(int id) const;
    int getOutDegree(int id) const;
    int getMaxId() const;
    int getTaskCount() const;
    int getDependencyCount() const;

    int findTaskById(int id) const;
    int findTaskByName(const char* nm) const;
    bool renameTask(int id, const char* nm);

    void displayPrerequisites(int id) const;
    void displayDependents(int id) const;
    void displayNoPrerequisites() const;
    void displayNoDependents() const;
    void displayIsolated() const;

    bool removeAllDependencies(int id);
    bool isConnected(int a, int b) const;
    void displayChain(int id, int d = 0) const;

    void addDependencyUnchecked(int a, int b);

private:
    Task t[MAX_TASKS];
    Node* adj[MAX_TASKS];
    int n;

    static void clearList(Node*& h);
    static bool delNode(Node*& h, int id);
    static void addEnd(Node*& h, int id);

    bool reach(int cur, int target, bool vis[]) const;
    bool dfs(int u, int st[]) const;
};
