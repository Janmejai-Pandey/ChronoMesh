#include <iostream>
#include <cstring>
#include "../include/Graph.h"
using namespace std;

Graph::Graph() {
    n = 0;
    for (int i = 0; i < MAX_TASKS; i++) {
        t[i].id = 0;
        t[i].name[0] = '\0';
        t[i].active = false;
        adj[i] = NULL;
    }
}

Graph::~Graph() {
    for (int i = 0; i < MAX_TASKS; i++)
        clearList(adj[i]);
}

void Graph::clearList(Node*& h) {
    while (h != NULL) {
        Node* temp = h;
        h = h->next;
        delete temp;
    }
}

void Graph::addEnd(Node*& h, int id) {
    Node* p = new Node;
    p->id = id;
    p->next = NULL;

    if (h == NULL) {
        h = p;
        return;
    }

    Node* c = h;
    while (c->next != NULL)
        c = c->next;
    c->next = p;
}

bool Graph::delNode(Node*& h, int id) {
    Node* prev = NULL;
    Node* c = h;

    while (c != NULL) {
        if (c->id == id) {
            if (prev == NULL)
                h = c->next;
            else
                prev->next = c->next;
            delete c;
            return true;
        }
        prev = c;
        c = c->next;
    }
    return false;
}

bool Graph::isValidTask(int id) const {
    return id >= 1 && id <= n && t[id - 1].active;
}

bool Graph::hasDependency(int a, int b) const {
    if (a < 1 || a > n)
        return false;

    for (Node* p = adj[a - 1]; p != NULL; p = p->next)
        if (p->id == b)
            return true;
    return false;
}

const char* Graph::getTaskName(int id) const {
    return isValidTask(id) ? t[id - 1].name : NULL;
}

const Node* Graph::getDependents(int id) const {
    return isValidTask(id) ? adj[id - 1] : NULL;
}

int Graph::getOutDegree(int id) const {
    if (!isValidTask(id))
        return 0;

    int count = 0;
    for (Node* p = adj[id - 1]; p != NULL; p = p->next)
        count++;
    return count;
}

int Graph::getInDegree(int id) const {
    if (!isValidTask(id))
        return 0;

    int count = 0;
    for (int i = 1; i <= n; i++)
        if (t[i - 1].active && hasDependency(i, id))
            count++;
    return count;
}

int Graph::getMaxId() const {
    return n;
}

int Graph::getTaskCount() const {
    int count = 0;
    for (int i = 0; i < n; i++)
        if (t[i].active)
            count++;
    return count;
}

int Graph::getDependencyCount() const {
    int count = 0;
    for (int i = 0; i < n; i++)
        if (t[i].active)
            for (Node* p = adj[i]; p != NULL; p = p->next)
                count++;
    return count;
}

int Graph::addTask(const char* nm) {
    if (nm == NULL || nm[0] == '\0') {
        cout << "Error: Empty task name." << endl;
        return -1;
    }

    if (n >= MAX_TASKS) {
        cout << "Error: Maximum task limit reached." << endl;
        return -1;
    }

    t[n].id = n + 1;
    strncpy(t[n].name, nm, NAME_LEN - 1);
    t[n].name[NAME_LEN - 1] = '\0';
    t[n].active = true;
    adj[n] = NULL;

    n++;
    return n;
}

bool Graph::removeTask(int id) {
    if (!isValidTask(id)) {
        cout << "Error: Task does not exist." << endl;
        return false;
    }

    removeAllDependencies(id);
    t[id - 1].active = false;
    return true;
}

void Graph::displayTasks() const {
    cout << "\n--- Active Tasks ---\n";
    bool found = false;

    for (int i = 0; i < n; i++) {
        if (t[i].active) {
            cout << "ID: " << t[i].id
                 << " | Name: " << t[i].name << endl;
            found = true;
        }
    }

    if (!found)
        cout << "No tasks available." << endl;
}

bool Graph::addDependency(int a, int b) {
    if (!isValidTask(a) || !isValidTask(b)) {
        cout << "Error: Invalid task ID." << endl;
        return false;
    }

    if (a == b) {
        cout << "Error: Self dependency not allowed." << endl;
        return false;
    }

    if (hasDependency(a, b)) {
        cout << "Error: Dependency already exists." << endl;
        return false;
    }

    if (wouldCreateCycle(a, b)) {
        cout << "Error: Circular dependency detected." << endl;
        return false;
    }

    addEnd(adj[a - 1], b);
    return true;
}

bool Graph::removeDependency(int a, int b) {
    if (!isValidTask(a) || !isValidTask(b)) {
        cout << "Error: Invalid task ID." << endl;
        return false;
    }

    if (!delNode(adj[a - 1], b)) {
        cout << "Error: Dependency not found." << endl;
        return false;
    }
    return true;
}

void Graph::displayGraph() const {
    cout << "\n--- Dependency Graph ---\n";
    for (int i = 0; i < n; i++) {
        if (!t[i].active)
            continue;

        cout << t[i].id << " (" << t[i].name << ") : ";
        for (Node* p = adj[i]; p != NULL; p = p->next)
            cout << p->id << " -> ";
        cout << "NULL" << endl;
    }
}

void Graph::addDependencyUnchecked(int a, int b) {
    if (a >= 1 && a <= n)
        addEnd(adj[a - 1], b);
}

bool Graph::reach(int cur, int target, bool vis[]) const {
    if (!isValidTask(cur))
        return false;
    if (cur == target)
        return true;

    vis[cur - 1] = true;

    for (Node* p = adj[cur - 1]; p != NULL; p = p->next) {
        int next = p->id;
        if (isValidTask(next) && !vis[next - 1])
            if (reach(next, target, vis))
                return true;
    }
    return false;
}

bool Graph::wouldCreateCycle(int a, int b) const {
    if (!isValidTask(a) || !isValidTask(b))
        return false;

    bool vis[MAX_TASKS] = {false};
    return reach(b, a, vis);
}

bool Graph::dfs(int u, int st[]) const {
    st[u - 1] = 1;

    for (Node* p = adj[u - 1]; p != NULL; p = p->next) {
        int v = p->id;
        if (!isValidTask(v))
            continue;

        if (st[v - 1] == 1)
            return true;

        if (st[v - 1] == 0 && dfs(v, st))
            return true;
    }

    st[u - 1] = 2;
    return false;
}

bool Graph::hasCycle() const {
    int st[MAX_TASKS] = {0};

    for (int i = 1; i <= n; i++)
        if (t[i - 1].active && st[i - 1] == 0 && dfs(i, st))
            return true;

    return false;
}

bool Graph::validateGraph() const {
    bool valid = true;

    for (int i = 0; i < n; i++) {
        if (!t[i].active) {
            if (adj[i] != NULL) {
                cout << "Deleted task has dependencies." << endl;
                valid = false;
            }
            continue;
        }

        for (Node* p = adj[i]; p != NULL; p = p->next) {
            int v = p->id;

            if (v < 1 || v > n) {
                cout << "Invalid dependency found." << endl;
                valid = false;
                continue;
            }

            if (!t[v - 1].active) {
                cout << "Dependency points to deleted task." << endl;
                valid = false;
            }

            if (v == i + 1) {
                cout << "Self dependency found." << endl;
                valid = false;
            }

            for (Node* q = p->next; q != NULL; q = q->next) {
                if (q->id == v) {
                    cout << "Duplicate dependency found." << endl;
                    valid = false;
                }
            }
        }
    }

    if (hasCycle()) {
        cout << "Circular dependency found." << endl;
        valid = false;
    }

    if (valid)
        cout << "Graph is valid." << endl;
    return valid;
}

int Graph::findTaskById(int id) const {
    return isValidTask(id) ? id - 1 : -1;
}

int Graph::findTaskByName(const char* nm) const {
    if (nm == NULL)
        return -1;

    for (int i = 0; i < n; i++)
        if (t[i].active && strcmp(t[i].name, nm) == 0)
            return t[i].id;
    return -1;
}

bool Graph::renameTask(int id, const char* nm) {
    if (!isValidTask(id) || nm == NULL || nm[0] == '\0')
        return false;

    strncpy(t[id - 1].name, nm, NAME_LEN - 1);
    t[id - 1].name[NAME_LEN - 1] = '\0';
    return true;
}

void Graph::displayPrerequisites(int id) const {
    if (!isValidTask(id)) {
        cout << "Error: Task not found." << endl;
        return;
    }

    cout << "Prerequisites of " << id << ": ";
    bool found = false;

    for (int i = 1; i <= n; i++) {
        if (t[i - 1].active && hasDependency(i, id)) {
            cout << i << "(" << t[i - 1].name << ") ";
            found = true;
        }
    }

    if (!found)
        cout << "None";
    cout << endl;
}

void Graph::displayDependents(int id) const {
    if (!isValidTask(id)) {
        cout << "Error: Task not found." << endl;
        return;
    }

    cout << "Dependents of " << id << ": ";
    if (adj[id - 1] == NULL)
        cout << "None";

    for (Node* p = adj[id - 1]; p != NULL; p = p->next)
        if (isValidTask(p->id))
            cout << p->id << "(" << t[p->id - 1].name << ") ";

    cout << endl;
}

void Graph::displayNoPrerequisites() const {
    cout << "Tasks without prerequisites: ";
    bool found = false;

    for (int i = 1; i <= n; i++) {
        if (t[i - 1].active && getInDegree(i) == 0) {
            cout << i << " ";
            found = true;
        }
    }

    if (!found)
        cout << "None";
    cout << endl;
}

void Graph::displayNoDependents() const {
    cout << "Tasks without dependents: ";
    bool found = false;

    for (int i = 1; i <= n; i++) {
        if (t[i - 1].active && adj[i - 1] == NULL) {
            cout << i << " ";
            found = true;
        }
    }

    if (!found)
        cout << "None";
    cout << endl;
}

void Graph::displayIsolated() const {
    cout << "Isolated tasks: ";
    bool found = false;

    for (int i = 1; i <= n; i++) {
        if (t[i - 1].active && adj[i - 1] == NULL && getInDegree(i) == 0) {
            cout << i << " ";
            found = true;
        }
    }

    if (!found)
        cout << "None";
    cout << endl;
}

bool Graph::removeAllDependencies(int id) {
    if (!isValidTask(id)) {
        cout << "Error: Task does not exist." << endl;
        return false;
    }

    clearList(adj[id - 1]);
    for (int i = 0; i < n; i++)
        delNode(adj[i], id);
    return true;
}

bool Graph::isConnected(int a, int b) const {
    if (!isValidTask(a) || !isValidTask(b))
        return false;

    bool v1[MAX_TASKS] = {false};
    bool v2[MAX_TASKS] = {false};
    return reach(a, b, v1) || reach(b, a, v2);
}

void Graph::displayChain(int id, int d) const {
    if (!isValidTask(id))
        return;

    for (int i = 0; i < d; i++)
        cout << "    ";

    cout << id << " (" << t[id - 1].name << ")" << endl;

    for (Node* p = adj[id - 1]; p != NULL; p = p->next)
        if (isValidTask(p->id))
            displayChain(p->id, d + 1);
}
