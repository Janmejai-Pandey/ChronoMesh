#pragma once
#include <string>

struct Task {
    int id;
    std::string name;
    int duration;   // time units
    int priority;   // higher number = more urgent
};