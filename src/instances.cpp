#include "instances.h"
#include "config.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <random>
#include <thread>

namespace {

constexpr int kChunkInstances = 4096;

std::uint32_t entropyWord() {
    try {
        static std::random_device rd;
        return rd();
    } catch (...) {
        static std::atomic<std::uint32_t> counter{0};

        static std::atomic<bool> warned{false};

        if (!warned.exchange(true)) {
            std::fprintf(stderr,
                "WARNING: std::random_device failed, falling back to "
                "reduced-entropy mix (time+counter+address)\n");
        }

        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                      std::chrono::steady_clock::now().time_since_epoch())
                      .count();

        std::uint32_t w = (std::uint32_t)ns;
        w ^= counter.fetch_add(0x9E3779B9u) * 0x85EBCA6Bu;
        w ^= (std::uint32_t)(uintptr_t)&w;
        w ^= w >> 16;
        w *= 0x2545F491u;
        w ^= w >> 15;
        return w;
    }
}

std::array<std::uint32_t, 4> entropyMaster() {
    return {entropyWord(), entropyWord(), entropyWord(), entropyWord()};
}

}

std::vector<Instance> buildInstancesSeeded(
    int n, const std::array<std::uint32_t, 4>& master) {

    if (n < 1)
        return {};

    std::vector<Instance> out((std::size_t)n);

    int side = (int)std::ceil(std::cbrt((double)n));
    float half = (side - 1) * 0.5f;
    int grid = side * side;

    unsigned chunks =
        (unsigned)(((std::size_t)n + kChunkInstances - 1) / kChunkInstances);

    unsigned hc = std::thread::hardware_concurrency();

    unsigned threads =
        std::max(1u, std::min(hc ? hc : 1u, chunks));

    auto worker = [&](unsigned start, unsigned stride) {
        for (unsigned c = start; c < chunks; c += stride) {
            std::array<std::uint32_t, 5> words = {
                master[0], master[1], master[2], master[3], c
            };

            std::seed_seq seq(words.begin(), words.end());
            std::mt19937 rng(seq);

            std::uniform_real_distribution<float> jitter(-0.25f, 0.25f);
            std::uniform_real_distribution<float> seedDist(0.0f, 1000000.0f);

            int begin = (int)c * kChunkInstances;
            int end = begin + kChunkInstances < n ? begin + kChunkInstances : n;

            for (int i = begin; i < end; ++i) {
                int z = i / grid;
                int rem = i % grid;
                int y = rem / side;
                int x = rem % side;

                out[i].x = (x - half) * SPACING + jitter(rng);
                out[i].y = (y - half) * SPACING + jitter(rng);
                out[i].z = (z - half) * SPACING + jitter(rng);
                out[i].seed = seedDist(rng);
            }
        }
    };

    std::vector<std::thread> pool;
    pool.reserve(threads);

    for (unsigned t = 0; t < threads; ++t)
        pool.emplace_back(worker, t, threads);

    for (auto& th : pool)
        th.join();

    return out;
}

std::vector<Instance> buildInstances(int n) {
    return buildInstancesSeeded(n, entropyMaster());
}
