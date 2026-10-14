#pragma once

#include <vector>

struct Instance {
    float x;
    float y;
    float z;
    float seed;
};

std::vector<Instance> buildInstances(int n);
