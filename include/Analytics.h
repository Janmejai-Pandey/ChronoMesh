#pragma once

#include "Execution.h"
#include "Scheduler.h"

void displayAnalytics(const Graph& graph,
                      const ExecutionResult& execution,
                      const CriticalPathResult& criticalPath);
