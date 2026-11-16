// Person 3 tests: execution engine (states, failures, blocked tasks, log).
#include "Execution.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

// Defined in src/execution/Execution.cpp. Declared here because Execution.h is
// kept unchanged for now - move this declaration into Execution.h later.
ExecutionResult runWithFailures(const Graph& graph, int workerCount,
                                const std::vector<int>& failTasks,
                                std::vector<std::string>& states,
                                std::vector<std::string>& log);

namespace {
// Dinner example:  Boil -> Rice -> Serve,  Chop -> Curry -> Serve,  Chop -> Salad -> Serve
Graph makeDinner() {
    Graph graph;
    bool ok = true;
    ok = ok && graph.addTask({0, "Boil", 4, 2});
    ok = ok && graph.addTask({1, "Chop", 3, 3});
    ok = ok && graph.addTask({2, "Rice", 5, 2});
    ok = ok && graph.addTask({3, "Curry", 6, 3});
    ok = ok && graph.addTask({4, "Salad", 2, 1});
    ok = ok && graph.addTask({5, "Serve", 1, 1});
    ok = ok && graph.addDependency(0, 2);
    ok = ok && graph.addDependency(1, 3);
    ok = ok && graph.addDependency(1, 4);
    ok = ok && graph.addDependency(2, 5);
    ok = ok && graph.addDependency(3, 5);
    ok = ok && graph.addDependency(4, 5);
    assert(ok);
    return graph;
}

int countState(const std::vector<std::string>& states, const std::string& name) {
    return static_cast<int>(std::count(states.begin(), states.end(), name));
}

void testNormalRun() {
    Graph graph = makeDinner();
    assert(ExecutionEngine(graph).run(2).totalTime == 12);
    assert(ExecutionEngine(graph).run(1).totalTime == 21);

    std::vector<std::string> states;
    std::vector<std::string> log;
    const ExecutionResult result = runWithFailures(graph, 2, {}, states, log);
    assert(result.valid && result.totalTime == 12);
    assert(countState(states, "DONE") == 6);
    assert(!log.empty());
    assert(log[0] == "t=0: Worker 0 started Chop (until t=3)");
}

void testOneFailureBlocksDependents() {
    Graph graph = makeDinner();
    std::vector<std::string> states;
    std::vector<std::string> log;
    const ExecutionResult result = runWithFailures(graph, 2, {0}, states, log);   // Boil fails
    assert(result.valid);
    assert(states[0] == "FAILED");    // Boil
    assert(states[2] == "BLOCKED");   // Rice needs Boil
    assert(states[5] == "BLOCKED");   // Serve needs Rice
    assert(states[1] == "DONE" && states[3] == "DONE" && states[4] == "DONE");
    assert(result.totalTime == 9);
    assert(result.tasks.size() == 4);   // blocked tasks never use a worker
}

void testLastTaskFails() {
    Graph graph = makeDinner();
    std::vector<std::string> states;
    std::vector<std::string> log;
    const ExecutionResult result = runWithFailures(graph, 2, {5}, states, log);   // Serve fails
    assert(result.valid && result.totalTime == 12);
    assert(countState(states, "DONE") == 5 && states[5] == "FAILED");
    assert(countState(states, "BLOCKED") == 0);
}

void testTwoFailures() {
    Graph graph = makeDinner();
    std::vector<std::string> states;
    std::vector<std::string> log;
    const ExecutionResult result = runWithFailures(graph, 2, {0, 1}, states, log);   // Boil and Chop fail
    assert(result.valid && result.totalTime == 4);
    assert(countState(states, "FAILED") == 2 && countState(states, "BLOCKED") == 4);
}

void testBadInput() {
    Graph graph = makeDinner();
    std::vector<std::string> states;
    std::vector<std::string> log;
    const ExecutionResult ignored = runWithFailures(graph, 2, {-1, 99}, states, log);   // unknown IDs ignored
    assert(ignored.valid && ignored.totalTime == 12 && countState(states, "DONE") == 6);

    assert(!ExecutionEngine(graph).run(0).valid);   // no workers

    Graph empty;
    assert(ExecutionEngine(empty).run(2).valid);

    Graph cycle = makeDinner();
    const bool added = cycle.addDependency(5, 0);   // Serve -> Boil makes a loop
    assert(added);
    assert(!ExecutionEngine(cycle).run(2).valid);
}

void showDemo() {
    Graph graph = makeDinner();
    std::vector<std::string> states;
    std::vector<std::string> log;
    runWithFailures(graph, 2, {0}, states, log);

    std::cout << "\nDemo: 2 workers, Boil fails\n";
    for (const std::string& line : log) {
        std::cout << "  " << line << '\n';
    }
    for (int id = 0; id < graph.size(); ++id) {
        std::cout << "  [" << id << ": " << graph.getTask(id).name << "] " << states[id] << '\n';
    }
}
}

int main() {
    testNormalRun();
    testOneFailureBlocksDependents();
    testLastTaskFails();
    testTwoFailures();
    testBadInput();
    std::cout << "All execution tests passed.\n";
    showDemo();
    return 0;
}
