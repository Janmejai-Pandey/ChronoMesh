#include "Benchmark.h"
#include "Execution.h"
#include "Graph.h"
#include "Scheduler.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace {
struct AppState {
    Graph graph;
    int page = 0;
    int selectedTaskId = -1;
    std::vector<ImVec2> graphOffsets;
    char taskName[128] = "";
    int taskDuration = 1;
    int taskPriority = 1;
    int dependencyFrom = -1;
    int dependencyTo = -1;
    int workerCount = 2;
    ExecutionResult execution;
    CriticalPathResult criticalPath;
    BenchmarkResult benchmark;
    bool hasExecution = false;
    bool hasBenchmark = false;
    std::string message;
    bool messageIsError = false;
};

bool pathExists(const Graph& graph, int start, int target) {
    if (start < 0 || target < 0 || start >= graph.size() || target >= graph.size()) {
        return false;
    }
    std::vector<bool> visited(graph.size(), false);
    std::vector<int> pending(1, start);
    while (!pending.empty()) {
        const int current = pending.back();
        pending.pop_back();
        if (current == target) {
            return true;
        }
        if (visited[current]) {
            continue;
        }
        visited[current] = true;
        for (int next : graph.getSuccessors(current)) {
            pending.push_back(next);
        }
    }
    return false;
}

void setMessage(AppState& state, const std::string& text, bool isError = false) {
    state.message = text;
    state.messageIsError = isError;
}

ImU32 priorityColor(int priority) {
    if (priority >= 8) return IM_COL32(255, 128, 112, 255);
    if (priority >= 4) return IM_COL32(255, 190, 92, 255);
    return IM_COL32(92, 196, 255, 255);
}

void drawMetric(const char* label, const std::string& value, const char* detail = nullptr) {
    ImGui::BeginChild(label, ImVec2(0.0f, 130.0f), true);
    ImGui::TextDisabled("%s", label);
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.58f, 0.78f, 1.0f, 1.0f), "%s", value.c_str());
    if (detail != nullptr) {
        ImGui::TextDisabled("%s", detail);
    }
    ImGui::EndChild();
}

void showTaskEditor(AppState& state) {
    ImGui::Text("TASKS");
    ImGui::TextDisabled("Create work items and set their priority.");
    ImGui::Spacing();

    ImGui::InputText("Name", state.taskName, sizeof(state.taskName));
    ImGui::InputInt("Duration", &state.taskDuration);
    ImGui::InputInt("Priority", &state.taskPriority);
    if (ImGui::Button("Add task", ImVec2(-1.0f, 34.0f))) {
        const Task task{state.graph.size(), state.taskName, state.taskDuration, state.taskPriority};
        if (state.graph.addTask(task)) {
            state.graphOffsets.push_back(ImVec2(0.0f, 0.0f));
            state.selectedTaskId = task.id;
            state.taskName[0] = '\0';
            state.hasExecution = false;
            state.hasBenchmark = false;
            setMessage(state, "Task added.");
        } else {
            setMessage(state, "Use a task name and a duration greater than zero.", true);
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("DEPENDENCIES");
    ImGui::TextDisabled("The first task must finish before the second starts.");

    if (state.graph.size() > 0) {
        const auto showTaskCombo = [&state](const char* label, int& selected) {
            char current[160];
            if (selected >= 0 && selected < state.graph.size()) {
                std::snprintf(current, sizeof(current), "%d - %s", selected,
                              state.graph.getTask(selected).name.c_str());
            } else {
                std::snprintf(current, sizeof(current), "Choose task");
            }
            if (ImGui::BeginCombo(label, current)) {
                for (int id = 0; id < state.graph.size(); ++id) {
                    char item[160];
                    std::snprintf(item, sizeof(item), "%d - %s", id,
                                  state.graph.getTask(id).name.c_str());
                    const bool isSelected = selected == id;
                    if (ImGui::Selectable(item, isSelected)) {
                        selected = id;
                    }
                    if (isSelected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        };
        showTaskCombo("Must finish first", state.dependencyFrom);
        showTaskCombo("Waits for it", state.dependencyTo);
        if (ImGui::Button("Add dependency", ImVec2(-1.0f, 34.0f))) {
            if (state.dependencyFrom < 0 || state.dependencyTo < 0) {
                setMessage(state, "Choose both tasks first.", true);
            } else if (state.dependencyFrom == state.dependencyTo) {
                setMessage(state, "A task cannot depend on itself.", true);
            } else if (pathExists(state.graph, state.dependencyTo, state.dependencyFrom)) {
                setMessage(state, "That dependency would create a cycle.", true);
            } else if (state.graph.addDependency(state.dependencyFrom, state.dependencyTo)) {
                state.hasExecution = false;
                state.hasBenchmark = false;
                setMessage(state, "Dependency added.");
            } else {
                setMessage(state, "Dependency already exists.", true);
            }
        }
    } else {
        ImGui::TextDisabled("Add at least one task first.");
    }

    if (!state.message.empty()) {
        ImGui::Spacing();
        const ImVec4 color = state.messageIsError
            ? ImVec4(1.0f, 0.43f, 0.38f, 1.0f)
            : ImVec4(0.38f, 0.84f, 0.59f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextWrapped("%s", state.message.c_str());
        ImGui::PopStyleColor();
    }
}

void showTaskList(AppState& state) {
    const Graph& graph = state.graph;
    ImGui::Text("TASK LIST");
    if (graph.size() == 0) {
        ImGui::TextDisabled("Your tasks will appear here.");
        return;
    }
    if (ImGui::BeginTable("tasks", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
                                      ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 34.0f);
        ImGui::TableSetupColumn("Task");
        ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed, 48.0f);
        ImGui::TableSetupColumn("Priority", ImGuiTableColumnFlags_WidthFixed, 58.0f);
        ImGui::TableHeadersRow();
        for (int id = 0; id < graph.size(); ++id) {
            const Task& task = graph.getTask(id);
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("%d", id);
            ImGui::TableSetColumnIndex(1);
            if (ImGui::Selectable(task.name.c_str(), state.selectedTaskId == id,
                                  ImGuiSelectableFlags_SpanAllColumns)) {
                state.selectedTaskId = id;
            }
            ImGui::TableSetColumnIndex(2); ImGui::Text("%d", task.duration);
            ImGui::TableSetColumnIndex(3); ImGui::TextColored(ImVec4(1.0f, 0.72f, 0.34f, 1.0f), "%d", task.priority);
        }
        ImGui::EndTable();
    }
}

void showDependencyGraph(AppState& state, float panelHeight = 0.0f) {
    const Graph& graph = state.graph;
    const ImVec2 available = ImGui::GetContentRegionAvail();
    const ImVec2 panelSize(0.0f, panelHeight > 0.0f ? panelHeight : available.y);
    ImGui::BeginChild("graph-canvas", panelSize, true, ImGuiWindowFlags_HorizontalScrollbar);
    const ImVec2 visible = ImGui::GetContentRegionAvail();
    if (graph.size() == 0) {
        ImGui::TextDisabled("Add tasks to see the dependency graph.");
        ImGui::EndChild();
        return;
    }

    const ScheduleResult topo = Scheduler(graph).topologicalSort();
    if (!topo.valid) {
        ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.4f, 1.0f), "This graph contains a cycle.");
        ImGui::EndChild();
        return;
    }

    std::vector<int> level(graph.size(), 0);
    int lastLevel = 0;
    int largestColumn = 1;
    for (int id : topo.order) {
        for (int next : graph.getSuccessors(id)) {
            level[next] = std::max(level[next], level[id] + 1);
            lastLevel = std::max(lastLevel, level[next]);
        }
    }
    std::vector<int> columnCounts(lastLevel + 1, 0);
    for (int stage : level) largestColumn = std::max(largestColumn, ++columnCounts[stage]);

    const float nodeW = 220.0f;
    const float nodeH = 100.0f;
    const float gapX = 104.0f;
    const float gapY = 34.0f;
    const float pad = 44.0f;
    const float canvasW = std::max(visible.x, pad * 2 + (lastLevel + 1) * nodeW + lastLevel * gapX);
    const float canvasH = std::max(visible.y, pad * 2 + largestColumn * nodeH + (largestColumn - 1) * gapY);
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(origin, ImVec2(origin.x + canvasW, origin.y + canvasH), IM_COL32(28, 34, 45, 255));
    for (float x = 0.0f; x < canvasW; x += 32.0f) {
        draw->AddLine(ImVec2(origin.x + x, origin.y), ImVec2(origin.x + x, origin.y + canvasH),
                      IM_COL32(54, 65, 81, 120));
    }
    for (float y = 0.0f; y < canvasH; y += 32.0f) {
        draw->AddLine(ImVec2(origin.x, origin.y + y), ImVec2(origin.x + canvasW, origin.y + y),
                      IM_COL32(54, 65, 81, 120));
    }

    if (state.graphOffsets.size() < static_cast<std::size_t>(graph.size())) {
        state.graphOffsets.resize(graph.size(), ImVec2(0.0f, 0.0f));
    }

    std::vector<int> row(graph.size(), 0);
    std::fill(columnCounts.begin(), columnCounts.end(), 0);
    for (int id = 0; id < graph.size(); ++id) row[id] = columnCounts[level[id]]++;
    std::vector<ImVec2> topLeft(graph.size());
    for (int id = 0; id < graph.size(); ++id) {
        const float columnHeight = columnCounts[level[id]] * nodeH + std::max(0, columnCounts[level[id]] - 1) * gapY;
        const float startY = (canvasH - columnHeight) * 0.5f;
        topLeft[id] = ImVec2(origin.x + pad + level[id] * (nodeW + gapX),
                             origin.y + startY + row[id] * (nodeH + gapY));
        topLeft[id].x += state.graphOffsets[id].x;
        topLeft[id].y += state.graphOffsets[id].y;
    }

    // Draw links first so task cards sit on top of the connector lines.
    for (int from = 0; from < graph.size(); ++from) {
        for (int to : graph.getSuccessors(from)) {
            const ImVec2 a(topLeft[from].x + nodeW, topLeft[from].y + nodeH * 0.5f);
            const ImVec2 b(topLeft[to].x, topLeft[to].y + nodeH * 0.5f);
            const float bend = std::max(24.0f, (b.x - a.x) * 0.45f);
            const bool linkedToSelection = state.selectedTaskId == from || state.selectedTaskId == to;
            draw->AddBezierCubic(a, ImVec2(a.x + bend, a.y), ImVec2(b.x - bend, b.y), b,
                                 linkedToSelection ? IM_COL32(76, 205, 151, 255) : IM_COL32(126, 151, 181, 230),
                                 linkedToSelection ? 3.5f : 2.5f);
            draw->AddTriangleFilled(ImVec2(b.x - 1.0f, b.y), ImVec2(b.x - 12.0f, b.y - 6.0f),
                                    ImVec2(b.x - 12.0f, b.y + 6.0f),
                                    linkedToSelection ? IM_COL32(76, 205, 151, 255) : IM_COL32(126, 151, 181, 255));
        }
    }

    for (int id = 0; id < graph.size(); ++id) {
        const Task& task = graph.getTask(id);
        ImVec2 p = topLeft[id];
        ImGui::SetCursorScreenPos(p);
        ImGui::PushID(id);
        ImGui::InvisibleButton("task-node", ImVec2(nodeW, nodeH));
        const bool hovered = ImGui::IsItemHovered();
        if (ImGui::IsItemClicked()) {
            state.selectedTaskId = id;
        }
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            const ImVec2 delta = ImGui::GetIO().MouseDelta;
            state.graphOffsets[id].x += delta.x;
            state.graphOffsets[id].y += delta.y;
            const float minX = pad;
            const float minY = pad;
            const float maxX = std::max(minX, canvasW - nodeW - pad);
            const float maxY = std::max(minY, canvasH - nodeH - pad);
            p.x = std::max(origin.x + minX, std::min(origin.x + maxX, p.x + delta.x));
            p.y = std::max(origin.y + minY, std::min(origin.y + maxY, p.y + delta.y));
            state.graphOffsets[id].x = p.x - (origin.x + pad + level[id] * (nodeW + gapX));
            state.graphOffsets[id].y = p.y - (origin.y + (canvasH - (columnCounts[level[id]] * nodeH +
                std::max(0, columnCounts[level[id]] - 1) * gapY)) * 0.5f + row[id] * (nodeH + gapY));
        }
        const bool selected = state.selectedTaskId == id;
        const bool related = state.selectedTaskId >= 0 && state.selectedTaskId < graph.size() &&
            (std::find(graph.getSuccessors(state.selectedTaskId).begin(),
                       graph.getSuccessors(state.selectedTaskId).end(), id) != graph.getSuccessors(state.selectedTaskId).end() ||
             std::find(graph.getPredecessors(state.selectedTaskId).begin(),
                       graph.getPredecessors(state.selectedTaskId).end(), id) != graph.getPredecessors(state.selectedTaskId).end());
        const ImU32 fill = selected ? IM_COL32(34, 63, 93, 255)
                                    : (related ? IM_COL32(34, 66, 56, 255)
                                               : (hovered ? IM_COL32(48, 59, 74, 255) : IM_COL32(37, 45, 57, 255)));
        draw->AddRectFilled(p, ImVec2(p.x + nodeW, p.y + nodeH), fill, 12.0f);
        const ImU32 border = selected ? IM_COL32(76, 165, 235, 255)
                                      : (related ? IM_COL32(68, 177, 127, 255) : IM_COL32(91, 108, 128, 255));
        draw->AddRect(p, ImVec2(p.x + nodeW, p.y + nodeH), border, 8.0f, 0,
                      selected ? 3.0f : 1.5f);
        draw->AddRectFilled(p, ImVec2(p.x + 6.0f, p.y + nodeH), priorityColor(task.priority), 8.0f,
                            ImDrawFlags_RoundCornersLeft);
        draw->AddCircleFilled(ImVec2(p.x + 20.0f, p.y + 22.0f), 5.5f, priorityColor(task.priority));
        draw->AddText(ImVec2(p.x + 34.0f, p.y + 14.0f), IM_COL32(235, 241, 248, 255), task.name.c_str());
        char details[96];
        std::snprintf(details, sizeof(details), "ID %d   |   %d units   |   priority %d",
                      id, task.duration, task.priority);
        draw->AddText(ImVec2(p.x + 15.0f, p.y + 64.0f), IM_COL32(171, 185, 203, 255), details);
        if (hovered) {
            ImGui::SetTooltip("Drag to rearrange. Click to inspect task %d.", id);
        }
        ImGui::PopID();
    }
    ImGui::SetCursorScreenPos(origin);
    ImGui::Dummy(ImVec2(canvasW, canvasH));
    ImGui::EndChild();
}

void showSelectedTask(AppState& state) {
    ImGui::Text("SELECTED TASK");
    const int id = state.selectedTaskId;
    if (id < 0 || id >= state.graph.size()) {
        ImGui::TextDisabled("Select a task from the table or click its graph node.");
        return;
    }
    const Task& task = state.graph.getTask(id);
    ImGui::Text("ID %d   |   %s   |   duration %d   |   priority %d",
                id, task.name.c_str(), task.duration, task.priority);
    ImGui::Text("Depends on:");
    const std::vector<int>& predecessors = state.graph.getPredecessors(id);
    if (predecessors.empty()) {
        ImGui::SameLine(); ImGui::TextDisabled("none");
    } else {
        ImGui::SameLine();
        for (std::size_t i = 0; i < predecessors.size(); ++i) {
            if (i != 0) ImGui::SameLine();
            const int other = predecessors[i];
            if (ImGui::SmallButton((state.graph.getTask(other).name + "##pred" + std::to_string(other)).c_str())) {
                state.selectedTaskId = other;
            }
        }
    }
    ImGui::Text("Blocks:");
    const std::vector<int>& successors = state.graph.getSuccessors(id);
    if (successors.empty()) {
        ImGui::SameLine(); ImGui::TextDisabled("none");
    } else {
        ImGui::SameLine();
        for (std::size_t i = 0; i < successors.size(); ++i) {
            if (i != 0) ImGui::SameLine();
            const int other = successors[i];
            if (ImGui::SmallButton((state.graph.getTask(other).name + "##next" + std::to_string(other)).c_str())) {
                state.selectedTaskId = other;
            }
        }
    }
}

void showTimeline(AppState& state) {
    const Graph& graph = state.graph;
    const ExecutionResult& result = state.execution;
    if (!state.hasExecution || !result.valid) {
        ImGui::TextDisabled("Run a simulation to see worker timelines.");
        return;
    }
    if (result.workerCount == 0) {
        ImGui::TextDisabled("There are no workers in this simulation.");
        return;
    }
    if (result.tasks.empty()) {
        ImGui::TextDisabled("No tasks were scheduled (total simulated time: 0).");
        return;
    }

    const float rowHeight = 88.0f;
    const float labelWidth = 150.0f;
    const float totalWidth = std::max(1.0f, ImGui::GetContentRegionAvail().x - labelWidth - 20.0f);
    const int totalTime = result.totalTime;
    const int tickStep = std::max(1, totalTime / 5 + (totalTime % 5 == 0 ? 0 : 1));
    ImDrawList* draw = ImGui::GetWindowDrawList();

    for (int worker = 0; worker < result.workerCount; ++worker) {
        const ImVec2 rowStart = ImGui::GetCursorScreenPos();
        ImGui::Text("Worker %d", worker);
        const ImVec2 barStart(rowStart.x + labelWidth, rowStart.y + 2.0f);
        draw->AddLine(ImVec2(barStart.x, barStart.y + 29.0f),
                      ImVec2(barStart.x + totalWidth, barStart.y + 29.0f), IM_COL32(91, 108, 130, 255), 1.5f);
        const auto drawTick = [&](int time) {
            const float x = barStart.x + totalWidth * static_cast<float>(time) / totalTime;
            draw->AddLine(ImVec2(x, barStart.y + 18.0f), ImVec2(x, barStart.y + 40.0f), IM_COL32(103, 121, 145, 210));
            char timeLabel[24];
            std::snprintf(timeLabel, sizeof(timeLabel), "%d", time);
            const float labelWidthPixels = ImGui::CalcTextSize(timeLabel).x;
            const float maxLabelX = std::max(barStart.x, barStart.x + totalWidth - labelWidthPixels);
            const float labelX = std::clamp(x - labelWidthPixels * 0.5f,
                                            barStart.x, maxLabelX);
            draw->AddText(ImVec2(labelX, barStart.y + 46.0f), IM_COL32(166, 181, 201, 255), timeLabel);
        };
        int tick = 0;
        while (tick < totalTime) {
            drawTick(tick);
            if (totalTime - tick <= tickStep) break;
            tick += tickStep;
        }
        drawTick(totalTime);
        for (const TaskExecution& taskRun : result.tasks) {
            if (taskRun.workerId != worker) continue;
            const Task& task = graph.getTask(taskRun.taskId);
            const float x0 = barStart.x + totalWidth * taskRun.startTime / static_cast<float>(totalTime);
            const float x1 = barStart.x + totalWidth * taskRun.finishTime / static_cast<float>(totalTime);
            const ImVec2 a(x0, barStart.y + 6.0f);
            const ImVec2 b(std::max(x0 + 12.0f, x1), barStart.y + 47.0f);
            ImGui::SetCursorScreenPos(a);
            ImGui::PushID(worker * graph.size() + taskRun.taskId);
            ImGui::InvisibleButton("timeline-task", ImVec2(b.x - a.x, b.y - a.y));
            const bool hovered = ImGui::IsItemHovered();
            if (ImGui::IsItemClicked()) state.selectedTaskId = taskRun.taskId;
            draw->AddRectFilled(a, b, priorityColor(task.priority), 6.0f);
            draw->AddRect(a, b, state.selectedTaskId == taskRun.taskId
                ? IM_COL32(25, 45, 72, 255) : IM_COL32(255, 255, 255, 150), 6.0f,
                0, state.selectedTaskId == taskRun.taskId ? 2.5f : 1.0f);
            if (b.x - a.x > 72.0f) {
                draw->AddText(ImVec2(a.x + 9.0f, a.y + 13.0f), IM_COL32(20, 28, 40, 255), task.name.c_str());
            }
            if (hovered) ImGui::SetTooltip("%s  |  %d to %d  |  click to inspect",
                                           task.name.c_str(), taskRun.startTime, taskRun.finishTime);
            ImGui::PopID();
        }
        ImGui::SetCursorScreenPos(rowStart);
        ImGui::Dummy(ImVec2(0.0f, rowHeight));
    }
    ImGui::TextDisabled("Time units   |   total simulated time: %d", result.totalTime);
}

void showAnalytics(const Graph& graph, const AppState& state) {
    if (!state.hasExecution) {
        ImGui::TextDisabled("Run a simulation to calculate performance metrics.");
        return;
    }
    int edges = 0;
    int work = 0;
    for (int id = 0; id < graph.size(); ++id) {
        edges += static_cast<int>(graph.getSuccessors(id).size());
        work += graph.getTask(id).duration;
    }
    if (!state.execution.valid) {
        ImGui::TextColored(ImVec4(1.0f, 0.48f, 0.42f, 1.0f),
                           "Simulation could not complete. Check the worker count and dependency graph.");
        return;
    }

    const double makespan = static_cast<double>(state.execution.totalTime);
    const double throughput = makespan > 0.0 ? graph.size() / makespan : 0.0;
    const double utilization = makespan > 0.0 && state.execution.workerCount > 0
        ? 100.0 * work / (state.execution.workerCount * makespan) : 0.0;
    char number[80];
    ImGui::Columns(3, "metrics", false);
    drawMetric("TASKS", std::to_string(graph.size()), "scheduled work items"); ImGui::NextColumn();
    drawMetric("DEPENDENCIES", std::to_string(edges), "direct task links"); ImGui::NextColumn();
    drawMetric("WORKERS", std::to_string(state.execution.workerCount), "parallel workers");
    ImGui::NextColumn();
    drawMetric("MAKESPAN", std::to_string(state.execution.totalTime), "time units to finish"); ImGui::NextColumn();
    drawMetric("WORKER UTILIZATION", (std::snprintf(number, sizeof(number), "%.1f%%", utilization), std::string(number)), "busy worker capacity"); ImGui::NextColumn();
    std::snprintf(number, sizeof(number), "%.2f", throughput);
    drawMetric("THROUGHPUT", number, "tasks per time unit");
    ImGui::Columns(1);

    ImGui::Spacing();
    ImGui::Text("Total task work: %d time units", work);
    ImGui::Text("Scheduler calculation: %lld microseconds", state.execution.calculationTimeMicroseconds);
    if (state.criticalPath.totalDuration >= 0) {
        ImGui::Text("Critical path duration: %d time units", state.criticalPath.totalDuration);
        std::string path;
        for (std::size_t index = 0; index < state.criticalPath.path.size(); ++index) {
            const int id = state.criticalPath.path[index];
            if (index != 0) path += "  >  ";
            path += std::to_string(id) + ": " + graph.getTask(id).name;
        }
        ImGui::TextWrapped("Critical path: %s", path.c_str());
    }
}

void showBenchmark(AppState& state) {
    if (ImGui::Button("Run benchmark (100 repetitions)", ImVec2(-1.0f, 36.0f))) {
        state.benchmark = benchmarkScheduler(state.graph, 100, state.workerCount);
        state.hasBenchmark = state.benchmark.repetitions > 0;
    }
    if (!state.hasBenchmark) {
        ImGui::TextDisabled("Measures average scheduler time for the current graph.");
        return;
    }
    ImGui::Text("Average over %d runs (microseconds)", state.benchmark.repetitions);
    ImGui::Separator();
    ImGui::Text("Topological sort       %.3f us", state.benchmark.topologicalSortAverageMicroseconds);
    ImGui::Text("Priority schedule      %.3f us", state.benchmark.priorityScheduleAverageMicroseconds);
    ImGui::Text("Critical path          %.3f us", state.benchmark.criticalPathAverageMicroseconds);
    ImGui::Text("Parallel simulation    %.3f us", state.benchmark.executionAverageMicroseconds);
}

void showSidebar(AppState& state) {
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    const float sideWidth = std::max(250.0f, std::min(360.0f, display.x * 0.18f));
    ImGui::SetNextWindowPos(ImVec2(12.0f, 12.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(sideWidth, display.y - 24.0f), ImGuiCond_Always);
    ImGui::Begin("ChronoMesh Menu", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                                              ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    ImGui::TextColored(ImVec4(0.48f, 0.72f, 1.0f, 1.0f), "CHRONOMESH");
    ImGui::TextDisabled("Dependency scheduler");
    ImGui::Separator();
    ImGui::Spacing();

    const char* pages[] = {"Dashboard", "Tasks", "Dependency Graph", "Worker Timeline", "Analytics", "Benchmark"};
    for (int page = 0; page < 6; ++page) {
        const bool isSelectedPage = state.page == page;
        if (isSelectedPage) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.22f, 0.47f, 0.72f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.27f, 0.53f, 0.79f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        }
        if (ImGui::Button(pages[page], ImVec2(-1.0f, 44.0f))) {
            state.page = page;
        }
        if (isSelectedPage) ImGui::PopStyleColor(3);
        ImGui::Spacing();
    }

    ImGui::Separator();
    ImGui::Text("Tasks: %d", state.graph.size());
    ImGui::Text("Dependencies: %d", [&state]() {
        int count = 0;
        for (int id = 0; id < state.graph.size(); ++id) {
            count += static_cast<int>(state.graph.getSuccessors(id).size());
        }
        return count;
    }());
    if (!state.message.empty()) {
        ImGui::Separator();
        const ImVec4 messageColor = state.messageIsError
            ? ImVec4(1.0f, 0.43f, 0.38f, 1.0f)
            : ImVec4(0.38f, 0.84f, 0.59f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, messageColor);
        ImGui::TextWrapped("%s", state.message.c_str());
        ImGui::PopStyleColor();
    }
    ImGui::End();
}

void pageTitle(const char* title, const char* subtitle) {
    ImGui::SetWindowFontScale(1.3f);
    ImGui::TextColored(ImVec4(0.53f, 0.76f, 1.0f, 1.0f), "%s", title);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::TextDisabled("%s", subtitle);
    ImGui::Separator();
    ImGui::Spacing();
}

void loadSampleWorkflow(AppState& state) {
    if (state.graph.size() != 0) {
        return;
    }
    state.graph.addTask({0, "Prepare", 3, 2});
    state.graph.addTask({1, "Download", 2, 5});
    state.graph.addTask({2, "Build", 4, 4});
    state.graph.addTask({3, "Test", 2, 3});
    state.graph.addTask({4, "Package", 1, 1});
    state.graph.addDependency(0, 2);
    state.graph.addDependency(1, 2);
    state.graph.addDependency(2, 3);
    state.graph.addDependency(3, 4);
    state.graphOffsets.assign(state.graph.size(), ImVec2(0.0f, 0.0f));
    state.selectedTaskId = 2;
    state.hasExecution = false;
    state.hasBenchmark = false;
    setMessage(state, "Sample workflow loaded. Open the graph to drag and select its nodes.");
}

void showDashboard(AppState& state) {
    pageTitle("Dashboard", "Overview of the task graph and latest simulation.");
    int dependencies = 0;
    for (int id = 0; id < state.graph.size(); ++id) {
        dependencies += static_cast<int>(state.graph.getSuccessors(id).size());
    }
    const std::string makespan = state.hasExecution && state.execution.valid
        ? std::to_string(state.execution.totalTime) : "--";
    ImGui::Columns(4, "dashboard-metrics", false);
    drawMetric("TASKS", std::to_string(state.graph.size()), "work items"); ImGui::NextColumn();
    drawMetric("DEPENDENCIES", std::to_string(dependencies), "direct links"); ImGui::NextColumn();
    drawMetric("WORKERS", std::to_string(state.workerCount), "selected workers"); ImGui::NextColumn();
    drawMetric("LAST MAKESPAN", makespan, "time units");
    ImGui::Columns(1);

    ImGui::Spacing();
    ImGui::Text("Quick actions");
    if (ImGui::Button("Manage tasks", ImVec2(170.0f, 44.0f))) state.page = 1;
    ImGui::SameLine();
    if (ImGui::Button("Open dependency graph", ImVec2(220.0f, 44.0f))) state.page = 2;
    ImGui::SameLine();
    if (ImGui::Button("Run simulation", ImVec2(190.0f, 44.0f))) {
        state.execution = ExecutionEngine(state.graph).run(state.workerCount);
        state.criticalPath = Scheduler(state.graph).criticalPath();
        state.hasExecution = true;
        state.page = 3;
    }
    ImGui::SameLine();
    if (state.graph.size() == 0) {
        if (ImGui::Button("Load sample workflow", ImVec2(220.0f, 44.0f))) {
            loadSampleWorkflow(state);
        }
    } else {
        ImGui::BeginDisabled();
        ImGui::Button("Sample workflow loaded", ImVec2(220.0f, 44.0f));
        ImGui::EndDisabled();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Task list");
    ImGui::BeginChild("dashboard-task-list", ImVec2(0.0f, 0.0f), true);
    showTaskList(state);
    ImGui::EndChild();
}

void showTasksPage(AppState& state) {
    pageTitle("Tasks and dependencies", "Add work items and connect prerequisites.");
    ImGui::Columns(2, "task-layout", false);
    ImGui::BeginChild("task-editor-panel", ImVec2(0.0f, 0.0f), true);
    showTaskEditor(state);
    ImGui::EndChild();
    ImGui::NextColumn();
    ImGui::BeginChild("task-table-panel", ImVec2(0.0f, 0.0f), true);
    showTaskList(state);
    ImGui::Spacing();
    ImGui::Separator();
    showSelectedTask(state);
    ImGui::EndChild();
    ImGui::Columns(1);
}

void showGraphPage(AppState& state) {
    pageTitle("Dependency graph", "Click a node to inspect it. Drag nodes to rearrange the graph.");
    if (ImGui::Button("Reset layout", ImVec2(140.0f, 34.0f))) {
        state.graphOffsets.assign(state.graph.size(), ImVec2(0.0f, 0.0f));
    }
    ImGui::SameLine();
    ImGui::TextDisabled("Blue = selected   Green = directly connected   Node stripe = priority");
    const float graphHeight = std::max(280.0f, ImGui::GetContentRegionAvail().y - 180.0f);
    showDependencyGraph(state, graphHeight);
    ImGui::Separator();
    showSelectedTask(state);
}

void simulate(AppState& state) {
    state.execution = ExecutionEngine(state.graph).run(state.workerCount);
    state.criticalPath = Scheduler(state.graph).criticalPath();
    state.hasExecution = true;
}

void showTimelinePage(AppState& state) {
    pageTitle("Worker timeline", "Click a task bar to select that task and inspect its graph links.");
    ImGui::SetNextItemWidth(240.0f);
    ImGui::SliderInt("Worker count", &state.workerCount, 1, 16);
    ImGui::SameLine();
    if (ImGui::Button("Simulate", ImVec2(150.0f, 0.0f))) simulate(state);
    ImGui::Spacing();
    const float timelineHeight = std::max(220.0f, ImGui::GetContentRegionAvail().y - 175.0f);
    ImGui::BeginChild("timeline-panel", ImVec2(0.0f, timelineHeight), true);
    showTimeline(state);
    ImGui::EndChild();
    ImGui::Spacing();
    showSelectedTask(state);
}

void showAnalyticsPage(AppState& state) {
    pageTitle("Execution analytics", "Utilization, throughput, makespan and critical path from the latest simulation.");
    ImGui::SetNextItemWidth(240.0f);
    ImGui::SliderInt("Worker count", &state.workerCount, 1, 16);
    ImGui::SameLine();
    if (ImGui::Button("Refresh analytics", ImVec2(190.0f, 0.0f))) simulate(state);
    ImGui::Spacing();
    ImGui::Separator();
    showAnalytics(state.graph, state);
}

void showBenchmarkPage(AppState& state) {
    pageTitle("Scheduler benchmark", "Compare average calculation times for scheduling algorithms.");
    ImGui::SetNextItemWidth(240.0f);
    ImGui::SliderInt("Worker count", &state.workerCount, 1, 16);
    ImGui::Spacing();
    showBenchmark(state);
}

void showWorkspace(AppState& state) {
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    const float sideWidth = std::max(250.0f, std::min(360.0f, display.x * 0.18f));
    ImGui::SetNextWindowPos(ImVec2(sideWidth + 24.0f, 12.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(std::max(360.0f, display.x - sideWidth - 36.0f), display.y - 24.0f), ImGuiCond_Always);
    ImGui::Begin("ChronoMesh Workspace", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                                                  ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    switch (state.page) {
    case 0: showDashboard(state); break;
    case 1: showTasksPage(state); break;
    case 2: showGraphPage(state); break;
    case 3: showTimelinePage(state); break;
    case 4: showAnalyticsPage(state); break;
    case 5: showBenchmarkPage(state); break;
    default: state.page = 0; showDashboard(state); break;
    }
    ImGui::End();
}

void applyStyle() {
    ImGui::StyleColorsDark();
    ImGui::GetIO().FontGlobalScale = 1.0f;
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.TabRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.WindowPadding = ImVec2(12.0f, 12.0f);
    style.FramePadding = ImVec2(9.0f, 7.0f);
    style.ItemSpacing = ImVec2(8.0f, 8.0f);
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.055f, 0.070f, 0.100f, 1.0f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.080f, 0.100f, 0.140f, 1.0f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.120f, 0.150f, 0.200f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.170f, 0.220f, 0.290f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.150f, 0.190f, 0.250f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.210f, 0.270f, 0.350f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.140f, 0.290f, 0.450f, 1.0f);
    colors[ImGuiCol_Separator] = ImVec4(0.220f, 0.260f, 0.320f, 1.0f);
    style.ScaleAllSizes(1.12f);
}
}

int main() {
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(1600, 1000, "ChronoMesh | Task Scheduler", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return 1;
    }
    glfwMaximizeWindow(window);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImFont* segoe = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/segoeui.ttf", 22.0f);
    if (segoe == nullptr) {
        ImFontConfig fontConfig;
        fontConfig.SizePixels = 22.0f;
        io.Fonts->AddFontDefault(&fontConfig);
    }
    applyStyle();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    AppState state;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        showSidebar(state);
        showWorkspace(state);

        ImGui::Render();
        int displayWidth = 0;
        int displayHeight = 0;
        glfwGetFramebufferSize(window, &displayWidth, &displayHeight);
        glViewport(0, 0, displayWidth, displayHeight);
        glClearColor(0.055f, 0.070f, 0.100f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
