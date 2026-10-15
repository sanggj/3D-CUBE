#pragma once

#include <array>
#include <cstdint>
#include <vector>

struct Instance {
    float x;
    float y;
    float z;
    float seed;
};

std::vector<Instance> buildInstances(int n);

std::vector<Instance> buildInstancesSeeded(
    int n, const std::array<std::uint32_t, 4>& master);
