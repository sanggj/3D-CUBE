#include "instances.h"
#include "config.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <random>
#include <thread>

std::vector<Instance> buildInstances(int n) {
    std::vector<Instance> out(n);

    int side = (int)std::ceil(std::cbrt((double)n));
    float half = (side - 1) * 0.5f;

    unsigned threads = std::max(
        1u,
        std::min(
            std::thread::hardware_concurrency(),
            std::max(1u, (unsigned)(n / 4096))
        )
    );

    int chunk = (n + (int)threads - 1) / (int)threads;

    std::random_device rd;

    auto worker = [&](int begin, int end, std::array<uint32_t, 4> seeds) {
        std::seed_seq seq(seeds.begin(), seeds.end());
        std::mt19937 rng(seq);

        std::uniform_real_distribution<float> jitter(-0.25f, 0.25f);
        std::uniform_real_distribution<float> seedDist(0.0f, 1000000.0f);

        for (int i = begin; i < end && i < n; ++i) {
            int z = i / (side * side);
            int rem = i % (side * side);
            int y = rem / side;
            int x = rem % side;

            out[i].x = (x - half) * SPACING + jitter(rng);
            out[i].y = (y - half) * SPACING + jitter(rng);
            out[i].z = (z - half) * SPACING + jitter(rng);
            out[i].seed = seedDist(rng);
        }
    };

    std::vector<std::thread> pool;

    for (unsigned t = 0; t < threads; ++t) {
        int begin = (int)t * chunk;
        int end = begin + chunk;

        std::array<uint32_t, 4> seeds = {
            rd(), rd(), rd(), (uint32_t)t
        };

        pool.emplace_back(worker, begin, end, seeds);
    }

    for (auto& th : pool)
        th.join();

    return out;
}
