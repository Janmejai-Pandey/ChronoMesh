#include <iostream>
#include <cstdlib>
#include "../include/Graph.h"
using namespace std;

int readInt(const char* msg) {
    int x;
    while (true) {
        cout << msg;
        if (cin >> x) {
            cin.ignore(10000, '\n');
            return x;
        }
        if (cin.eof())
            exit(0);
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Invalid input. Enter a number." << endl;
    }
}

void readName(const char* msg, char* buf) {
    cout << msg;
    cin.getline(buf, NAME_LEN);

    if (cin.eof())
        exit(0);

    if (cin.fail()) {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Name was too long. Extra characters removed." << endl;
    }
}

void showMenu() {
    cout << "\n ChronoMesh : Graph Module \n"
         << "1. Add task             2. Remove task\n"
         << "3. Add dependency       4. Remove dependency\n"
         << "5. Show tasks           6. Show graph\n"
         << "7. Validate graph       8. Show prerequisites\n"
         << "9. Show dependents     10. Show chain\n"
         << "11. Statistics         12. Find task by name\n"
         << "13. Rename task        14. Remove all dependencies\n"
         << "15. Check connectivity\n"
         << "0. Exit\n";
}

int main() {
    Graph g;
    char nm[NAME_LEN];
    int ch, a, b, id;

    do {
        showMenu();
        ch = readInt("Choice: ");

        switch (ch) {
        case 1:
            readName("Task name: ", nm);
            id = g.addTask(nm);
            if (id != -1)
                cout << "Task added with ID " << id << endl;
            break;

        case 2:
            if (g.removeTask(readInt("Task ID: ")))
                cout << "Task removed." << endl;
            break;

        case 3:
            a = readInt("From (must finish first): ");
            b = readInt("To (waits for it): ");
            if (g.addDependency(a, b))
                cout << "Dependency added." << endl;
            break;

        case 4:
            a = readInt("From: ");
            b = readInt("To: ");
            if (g.removeDependency(a, b))
                cout << "Dependency removed." << endl;
            break;

        case 5:
            g.displayTasks();
            break;

        case 6:
            g.displayGraph();
            break;

        case 7:
            g.validateGraph();
            break;

        case 8:
            g.displayPrerequisites(readInt("Task ID: "));
            break;

        case 9:
            g.displayDependents(readInt("Task ID: "));
            break;

        case 10:
            g.displayChain(readInt("Task ID: "));
            break;

        case 11:
            cout << "Tasks: " << g.getTaskCount()
                 << " | Dependencies: " << g.getDependencyCount() << endl;
            g.displayNoPrerequisites();
            g.displayNoDependents();
            g.displayIsolated();
            break;

        case 12:
            readName("Name to find: ", nm);
            id = g.findTaskByName(nm);
            if (id == -1)
                cout << "Not found." << endl;
            else
                cout << "Found: ID " << id << endl;
            break;

        case 13:
            id = readInt("Task ID: ");
            readName("New name: ", nm);
            if (g.renameTask(id, nm))
                cout << "Renamed." << endl;
            break;

        case 14:
            if (g.removeAllDependencies(readInt("Task ID: ")))
                cout << "All dependencies removed." << endl;
            break;

        case 15:
            a = readInt("Task A: ");
            b = readInt("Task B: ");
            cout << (g.isConnected(a, b) ? "Connected." : "Not connected.") << endl;
            break;

        case 0:
            cout << "Goodbye!" << endl;
            break;

        default:
            cout << "Unknown option." << endl;
        }
    } while (ch != 0);

    return 0;
}
