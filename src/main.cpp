#include <iostream>
#include <limits>
#include <stdexcept>
#include "../include/Graph.h"

using namespace std;

void showMenu() {
    cout << "\n ChronoMesh Task Scheduler \n";
    cout << "1. Add Task\n";
    cout << "2. Add Dependency\n";
    cout << "3. Display All Tasks\n";
    cout << "4. Display Dependency Graph\n";
    cout << "5. Check Circular Dependency\n";
    cout << "6. View Task Details\n";
    cout << "0. Exit\n";
    cout << "Enter choice: ";
}

void addTask(Graph& g) {
    Task t;

    t.id = g.size();

    cout << "\nEnter Task Name: ";
    getline(cin >> ws, t.name);

    if (t.name.empty()) {
        cout << "Task name cannot be empty.\n";
        return;
    }

    cout << "Enter Duration: ";

    if (!(cin >> t.duration) || t.duration <= 0) {
        cout << "Invalid duration.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    cout << "Enter Priority (higher number = more urgent): ";

    if (!(cin >> t.priority) || t.priority < 0) {
        cout << "Invalid priority.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    if (g.addTask(t))
        cout << "Task added successfully! ID: " << t.id << endl;
    else
        cout << "Unable to add task.\n";
}

void addDependency(Graph& g) {
    int from, to;

    cout << "\nEnter prerequisite Task ID: ";

    if (!(cin >> from)) {
        cout << "Invalid input.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    cout << "Enter dependent Task ID: ";

    if (!(cin >> to)) {
        cout << "Invalid input.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    if (g.addDependency(from, to))
        cout << "Dependency added successfully.\n";
    else
        cout << "Invalid dependency or circular dependency detected.\n";
}

void displayTasks(const Graph& g) {
    vector<int> ids = g.getAllTaskIds();

    if (ids.empty()) {
        cout << "\nNo tasks available.\n";
        return;
    }

    cout << "\n========== ALL TASKS ==========\n";

    for (int i = 0; i < (int)ids.size(); i++) {
        const Task& t = g.getTask(ids[i]);

        cout << "\nTask ID: " << t.id;
        cout << "\nName: " << t.name;
        cout << "\nDuration: " << t.duration;
        cout << "\nPriority: " << t.priority << endl;
    }
}

void displayGraph(const Graph& g) {
    vector<int> ids = g.getAllTaskIds();

    if (ids.empty()) {
        cout << "\nNo tasks available.\n";
        return;
    }

    cout << "\n======= DEPENDENCY GRAPH =======\n";

    for (int i = 0; i < (int)ids.size(); i++) {
        int id = ids[i];

        cout << id << " (" << g.getTask(id).name << ") -> ";

        const vector<int>& next = g.getSuccessors(id);

        if (next.empty())
            cout << "None";

        for (int j = 0; j < (int)next.size(); j++)
            cout << next[j] << " ";

        cout << endl;
    }
}

void viewTask(const Graph& g) {
    int id;

    cout << "Enter Task ID: ";

    if (!(cin >> id)) {
        cout << "Invalid input.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    try {
        const Task& t = g.getTask(id);

        cout << "\n======= TASK DETAILS =======\n";
        cout << "ID: " << t.id << endl;
        cout << "Name: " << t.name << endl;
        cout << "Duration: " << t.duration << endl;
        cout << "Priority: " << t.priority << endl;

        cout << "\nPrerequisites: ";

        const vector<int>& pre = g.getPredecessors(id);

        if (pre.empty())
            cout << "None";

        for (int i = 0; i < (int)pre.size(); i++)
            cout << pre[i] << " ";

        cout << "\nDependents: ";

        const vector<int>& next = g.getSuccessors(id);

        if (next.empty())
            cout << "None";

        for (int i = 0; i < (int)next.size(); i++)
            cout << next[i] << " ";

        cout << endl;
    }
    catch (const out_of_range&) {
        cout << "Task ID not found.\n";
    }
}

int main() {
    Graph g;
    int choice;

    do {
        showMenu();

        if (!(cin >> choice)) {
            cout << "Invalid input. Enter a number.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        switch (choice) {
        case 1:
            addTask(g);
            break;

        case 2:
            addDependency(g);
            break;

        case 3:
            displayTasks(g);
            break;

        case 4:
            displayGraph(g);
            break;

        case 5:
            if (g.hasCycle())
                cout << "Circular dependency detected!\n";
            else
                cout << "No circular dependency found.\n";
            break;

        case 6:
            viewTask(g);
            break;

        case 0:
            cout << "Exiting ChronoMesh. Goodbye!\n";
            break;

        default:
            cout << "Invalid choice.\n";
        }

    } while (choice != 0);

    return 0;
}
