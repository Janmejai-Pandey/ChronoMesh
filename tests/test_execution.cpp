// Person 3 tests: execution engine (states, failures, blocked tasks, log).
#include "Execution.h"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

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

void testNormalRun() {
    Graph graph = makeDinner();
    const ExecutionResult two = ExecutionEngine(graph).run(2);
    assert(two.valid);
    assert(two.totalTime == 12);
    assert(two.doneCount == 6 && two.failedCount == 0 && two.blockedCount == 0);
    assert(two.states.size() == 6);
    for (TaskState state : two.states) {
        assert(state == TaskState::Done);
    }
    for (const TaskExecution& row : two.tasks) {
        assert(row.status == TaskState::Done);
    }
    assert(!two.log.empty());
    assert(two.log[0] == "t=0: Worker 0 started Chop (until t=3)");

    const ExecutionResult one = ExecutionEngine(graph).run(1);
    assert(one.valid && one.totalTime == 21);
}

void testOneFailureBlocksDependents() {
    Graph graph = makeDinner();
    const ExecutionResult result = ExecutionEngine(graph).run(2, {0});   // Boil fails
    assert(result.valid);
    assert(result.states[0] == TaskState::Failed);    // Boil
    assert(result.states[2] == TaskState::Blocked);   // Rice needs Boil
    assert(result.states[5] == TaskState::Blocked);   // Serve needs Rice
    assert(result.states[1] == TaskState::Done);      // Chop
    assert(result.states[3] == TaskState::Done);      // Curry
    assert(result.states[4] == TaskState::Done);      // Salad
    assert(result.doneCount == 3 && result.failedCount == 1 && result.blockedCount == 2);
    assert(result.totalTime == 9);
    assert(result.tasks.size() == 4);                 // blocked tasks never use a worker

    bool boilRowFailed = false;
    for (const TaskExecution& row : result.tasks) {
        if (row.taskId == 0) {
            boilRowFailed = (row.status == TaskState::Failed);
        }
    }
    assert(boilRowFailed);
}

void testLastTaskFails() {
    Graph graph = makeDinner();
    const ExecutionResult result = ExecutionEngine(graph).run(2, {5});   // Serve fails
    assert(result.valid);
    assert(result.states[5] == TaskState::Failed);
    assert(result.doneCount == 5 && result.failedCount == 1 && result.blockedCount == 0);
    assert(result.totalTime == 12);
}

void testTwoFailures() {
    Graph graph = makeDinner();
    const ExecutionResult result = ExecutionEngine(graph).run(2, {0, 1});   // Boil and Chop fail
    assert(result.valid);
    assert(result.doneCount == 0 && result.failedCount == 2 && result.blockedCount == 4);
    assert(result.totalTime == 4);
}

void testBadInput() {
    Graph graph = makeDinner();
    const ExecutionResult ignored = ExecutionEngine(graph).run(2, {-1, 99});   // unknown IDs are ignored
    assert(ignored.valid && ignored.totalTime == 12 && ignored.failedCount == 0);

    assert(!ExecutionEngine(graph).run(0).valid);   // no workers

    Graph empty;
    const ExecutionResult none = ExecutionEngine(empty).run(2);
    assert(none.valid && none.totalTime == 0);

    Graph cycle = makeDinner();
    const bool added = cycle.addDependency(5, 0);   // Serve -> Boil makes a loop
    assert(added);
    assert(!ExecutionEngine(cycle).run(2).valid);
}

void testToString() {
    assert(std::string(toString(TaskState::Done)) == "DONE");
    assert(std::string(toString(TaskState::Blocked)) == "BLOCKED");
}
}

int main() {
    testNormalRun();
    testOneFailureBlocksDependents();
    testLastTaskFails();
    testTwoFailures();
    testBadInput();
    testToString();
    std::cout << "All execution tests passed.\n";
    return 0;
}
