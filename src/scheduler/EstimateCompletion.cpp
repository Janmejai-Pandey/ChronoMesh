#include "Scheduler.h"

#include "Execution.h"

int Scheduler::estimateCompletion(int numWorkers) const {
    if (numWorkers <= 0) {
        return -1;
    }

    const ExecutionResult result = ExecutionEngine(graph).run(numWorkers);
    return result.valid ? result.totalTime : -1;
}
