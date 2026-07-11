#pragma once
#include "base_generator.h"
#include <math.h>

class NoiseGen : public BaseGenerator {
public:
    void generate(uint16_t* buffer, int width, int height, uint32_t frame_num) override {
        float t = frame_num * 0.03f;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                // Basit value noise (hash tabanlı)
                float nx = x * 0.02f + t;
                float ny = y * 0.02f + t * 0.5f;
                float v = noise2d(nx, ny);
                float v2 = noise2d(nx * 2.0f, ny * 2.0f) * 0.5f;
                float v3 = noise2d(nx * 4.0f, ny * 4.0f) * 0.25f;
                v = (v + v2 + v3) / 1.75f;
                v = (v + 1.0f) * 0.5f;  // 0..1

                uint8_t r = (uint8_t)(v * 100);
                uint8_t g = (uint8_t)(v * 200 + 55);
                uint8_t b = (uint8_t)(v * 150 + 50);
                buffer[y * width + x] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
            }
        }
    }
    const char* name() override { return "noise"; }

private:
    // Basit hash tabanlı 2D noise
    static float hash(int x, int y) {
        int n = x + y * 57;
        n = (n << 13) ^ n;
        return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
    }

    static float smoothNoise(float x, float y) {
        int ix = (int)floorf(x);
        int iy = (int)floorf(y);
        float fx = x - ix;
        float fy = y - iy;
        fx = fx * fx * (3.0f - 2.0f * fx);  // smoothstep
        fy = fy * fy * (3.0f - 2.0f * fy);
        float a = hash(ix, iy);
        float b = hash(ix + 1, iy);
        float c = hash(ix, iy + 1);
        float d = hash(ix + 1, iy + 1);
        return a + fx * (b - a) + fy * (c - a) + fx * fy * (a - b - c + d);
    }

    static float noise2d(float x, float y) {
        return smoothNoise(x, y);
    }
};
