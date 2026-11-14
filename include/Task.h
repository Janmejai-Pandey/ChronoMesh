#pragma once

const int MAX_TASKS = 100;
const int NAME_LEN = 50;

struct Task {
    int id;
    char name[NAME_LEN];
    bool active;
};
