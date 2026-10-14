#include "../src/mat4.h"
#include "../src/cube_geometry.h"

#include <cmath>
#include <cstdio>

static int failures = 0;

#define CHECK(cond)                                        \
    do {                                                   \
        if (!(cond)) {                                     \
            std::printf("FAIL %s:%d: %s\n",                \
                        __FILE__, __LINE__, #cond);        \
            ++failures;                                    \
        }                                                  \
    } while (0)

static bool near3(
    float a, float b, float c,
    float x, float y, float z,
    float eps = 1e-4f
) {
    return std::fabs(a - x) < eps &&
           std::fabs(b - y) < eps &&
           std::fabs(c - z) < eps;
}

static void apply(
    const Mat4& m,
    float x, float y, float z,
    float& ox, float& oy, float& oz,
    float& ow
) {
    ox = m.m[0]*x + m.m[4]*y + m.m[8]*z  + m.m[12];
    oy = m.m[1]*x + m.m[5]*y + m.m[9]*z  + m.m[13];
    oz = m.m[2]*x + m.m[6]*y + m.m[10]*z + m.m[14];
    ow = m.m[3]*x + m.m[7]*y + m.m[11]*z + m.m[15];
}

static void testWinding() {
    for (int t = 0; t < 36; t += 3) {
        const std::uint32_t i0 = kCubeIndices[t];
        const std::uint32_t i1 = kCubeIndices[t + 1];
        const std::uint32_t i2 = kCubeIndices[t + 2];

        if (i0 > 7 || i1 > 7 || i2 > 7) {
            CHECK(false);
            continue;
        }

        const float* a = &kCubeVerts[i0 * 3];
        const float* b = &kCubeVerts[i1 * 3];
        const float* c = &kCubeVerts[i2 * 3];

        const float e1[3] = {b[0]-a[0], b[1]-a[1], b[2]-a[2]};
        const float e2[3] = {c[0]-a[0], c[1]-a[1], c[2]-a[2]};

        const float n[3] = {
            e1[1]*e2[2] - e1[2]*e2[1],
            e1[2]*e2[0] - e1[0]*e2[2],
            e1[0]*e2[1] - e1[1]*e2[0]
        };

        const float centroid[3] = {
            (a[0]+b[0]+c[0]) / 3.0f,
            (a[1]+b[1]+c[1]) / 3.0f,
            (a[2]+b[2]+c[2]) / 3.0f
        };

        const float dot =
            n[0]*centroid[0] + n[1]*centroid[1] + n[2]*centroid[2];

        CHECK(dot > 0.0f);
    }
}

static void testCoverage() {
    bool used[8] = {};

    for (int t = 0; t < 36; ++t) {
        CHECK(kCubeIndices[t] < 8);
        if (kCubeIndices[t] < 8)
            used[kCubeIndices[t]] = true;
    }

    for (int v = 0; v < 8; ++v)
        CHECK(used[v]);

    CHECK(sizeof(kCubeVerts) / sizeof(kCubeVerts[0]) == 24);
    CHECK(sizeof(kCubeIndices) / sizeof(kCubeIndices[0]) == 36);
}

static void testIdentity() {
    Mat4 I = identity();
    Mat4 P = perspective(70.0f, 16.0f / 9.0f, 0.1f, 10000.0f);
    Mat4 R = mul(P, I);

    for (int i = 0; i < 16; ++i)
        CHECK(R.m[i] == P.m[i]);
}

static void testMulOrder() {
    Mat4 P = perspective(70.0f, 1.5f, 0.1f, 1000.0f);
    Mat4 V = translate(0.0f, 0.0f, -120.0f);
    Mat4 M = mul(rotY(0.7f), rotX(-0.3f));

    Mat4 MVP = mul(P, mul(V, M));

    float px, py, pz, pw;
    apply(MVP, 1.0f, 2.0f, -3.0f, px, py, pz, pw);

    float x, y, z, w;
    apply(M, 1.0f, 2.0f, -3.0f, x, y, z, w);
    apply(V, x, y, z, x, y, z, w);
    apply(P, x, y, z, x, y, z, w);

    CHECK(near3(px, py, pz, x, y, z));
    CHECK(std::fabs(pw - w) < 1e-4f);
}

static void testPerspectiveDepth() {
    Mat4 P = perspective(70.0f, 1.777f, 0.1f, 1000.0f);

    float x, y, z, w;
    apply(P, 0.0f, 0.0f, -0.1f, x, y, z, w);
    CHECK(std::fabs(z / w - (-1.0f)) < 1e-4f);

    apply(P, 0.0f, 0.0f, -1000.0f, x, y, z, w);
    CHECK(std::fabs(z / w - 1.0f) < 1e-4f);
}

static void testRotations() {
    float x, y, z, w;

    Mat4 T = translate(1.0f, 2.0f, 3.0f);
    apply(T, 0.0f, 0.0f, 0.0f, x, y, z, w);
    CHECK(near3(x, y, z, 1.0f, 2.0f, 3.0f));

    const float halfPi = 1.57079632679f;

    Mat4 Ry = rotY(halfPi);
    apply(Ry, 1.0f, 0.0f, 0.0f, x, y, z, w);
    CHECK(near3(x, y, z, 0.0f, 0.0f, -1.0f));

    Mat4 Rx = rotX(halfPi);
    apply(Rx, 0.0f, 1.0f, 0.0f, x, y, z, w);
    CHECK(near3(x, y, z, 0.0f, 0.0f, 1.0f));
}

int main() {
    testWinding();
    testCoverage();
    testIdentity();
    testMulOrder();
    testPerspectiveDepth();
    testRotations();

    if (failures) {
        std::printf("%d FAILURE(S)\n", failures);
        return 1;
    }

    std::printf("ALL TESTS PASSED\n");
    return 0;
}
