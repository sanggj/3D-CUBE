# CubeChaos

Instanced 3D cube field: up to 20 million CPU-generated cube instances drawn
in one `glDrawElementsInstanced` call, animated and colored in the vertex
shader. OpenGL 4.6 core + GLFW + Dear ImGui.

## Layout

- `src/` - application code
- `lib/` - vendored libraries (glad, glfw, imgui)
- `test/` - automated tests
- `build/`, `build-debug/` - build output (ignored by git)

## Build

Requires CMake >= 3.16 and MSVC (Visual Studio).

```
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Use `-DCMAKE_BUILD_TYPE=Debug` for debugging builds. Release (`/O2`) is the
performance configuration; on the measured machine Debug and Release ran at
identical FPS for both AA settings (the workload was not bound by
application code), but Release is the supported configuration.

Own code compiles at `/W4` (MSVC) or `-Wall -Wextra`; vendored library
sources are compiled with warnings disabled.

Run `build\CubeChaos.exe`.

## Test

```
ctest --test-dir build --output-on-failure
```

One CTest entry (`CubeGeometryAndMath`) runs the `CubeTests` executable.
Scope: cube face winding vs backface culling, index/vertex coverage, Mat4
math (multiply order, perspective depth mapping, rotations), and RNG
behavior: seeded determinism (bit-identical output for identical master
seed), stream separation (different masters diverge), placement invariants
(jitter bounds, seed range), and 20 repeated entropy-seeded rebuild cycles.
Tests are pure CPU, need no GUI, and fail the process exit code on any
failed check.

## Randomness

- Every build of the instance field draws fresh entropy from
  `std::random_device` (OS-backed on MSVC). There are no fixed seeds and no
  fixed-seed settings.
- If `std::random_device` throws, generation falls back to a documented
  reduced-entropy mix (steady_clock + atomic counter + address) and prints
  a one-time stderr warning. It never silently reverts to a fixed seed.
- Instances are generated in fixed 4096-instance chunks. Each chunk owns a
  private `std::mt19937` seeded by a `std::seed_seq` of {4 master words,
  chunk index}. No generator state is shared between threads, and chunk
  contents do not depend on thread count or scheduling.
- `mt19937` is deterministic and statistically strong, not cryptographic.
  The seeded streams are not proven mathematically independent.
- Deterministic mode (opt-in, debug only): set `CUBECHAOS_SEED=<number>`
  before launch. Layout becomes a pure function of that number, across
  launches and rebuilds. On-screen animation stays time-driven, and the
  GPU-side `hash()` uses `sin`-based math whose results can differ between
  GPUs/drivers, and `std::uniform_real_distribution` sequences are
  implementation-defined, so a seed reproduces the layout only under the
  same toolchain and GPU family, not as a cross-platform guarantee.
  Deterministic mode guarantees the CPU-side layout, not bit-identical
  frames.

## Controls

- Drag: orbit camera (pitch clamped just short of +/-90 deg)
- Scroll: zoom (clamped to keep the camera inside near/far range)
- Settings panel: cube count (max 20,000,000), instance spacing, culling,
  wireframe, VSync, runtime MSAA (0/2/4/8 via multisampled FBO + blit),
  camera reset
- Live stats: FPS, frame EMA, frame-time p50/p95/p99/max over the last 512
  frames, GPU scene time (clear through MSAA resolve, read from deferred
  timestamp queries - never stalls the pipeline), instance count
- `H`: hide/show the settings panel. Hidden mode skips all ImGui work per
  frame. While hidden, the window title shows `UI hidden - press H` so the
  mode is always discoverable and recoverable.

## Measured performance (one machine, workload-specific - not universal)

Hardware: RTX 3050 6GB laptop GPU, driver 616.56, OpenGL 4.6, 16 logical
CPU cores, Windows, 1280x720 windowed (DWM-composed), 34356 instances,
VSync off, Release.

Method: interleaved A/B trials against the previous build on the same
desktop, per-process CPU time deltas, in-app percentiles. The desktop is
shared and external load moved the ceiling during measurement (identical
configs measured 534-639 FPS @4x and 1552-1713 FPS @0x across the day);
paired back-to-back runs agree within ~1%, so only paired comparisons are
meaningful.

| Workload | Result |
|---|---|
| 4x MSAA, UI shown | ~624 FPS (GPU fill-bound; GPU time ~2.7ms > frame budget) |
| AA off, UI shown | ~1552-1713 FPS depending on external load; CPU ~0.75 ms/frame |
| AA off, UI hidden | ~1766 FPS; CPU ~0.17-0.25 ms/frame |
| Rebuild 34356 / 2M / 20M instances | 1.2ms / 19.5ms build + 5.9ms / 108.7ms build + 46.4ms upload |

At 4x MSAA the GPU is the bottleneck; hiding the UI frees CPU but does not
raise FPS. With AA off the application is limited by GL driver submission
and presentation; the remaining CPU cost is not application logic
(Debug and Release performed identically before these changes).

## Known limits and unverified claims

- Frame rates above assume this GPU at this resolution; fullscreen
  exclusive mode was not tested.
- GPU utilization counters were unreliable in this environment; bottleneck
  classification rests on behavioral evidence (AA on/off deltas, UI-hide
  deltas, timer-query GPU times), not on utilization percentages.
- Thermal/power behavior at sustained 1700+ FPS was not characterized.
- Rebuild at 20M instances is a one-time ~150ms hitch (generate +
  320MB upload) on the render thread; the app stays responsive enough to
  continue afterward.
