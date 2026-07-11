#pragma once
#include "base_generator.h"
#include <math.h>

class NoiseGen : public BaseGenerator {
public:
    void generate(uint16_t* buffer, int width, int height, uint32_t frame_num) override {
        // 600x600'de performansı korumak için 3x3 bloklar halinde (200x200 mantıksal) çiziyoruz.
        int scale = 3;
        int logicalW = width / scale;
        int logicalH = height / scale;

        float t = frame_num * 0.04f;
        for (int y = 0; y < logicalH; y++) {
            for (int x = 0; x < logicalW; x++) {
                // Basit value noise (hash tabanlı)
                float nx = x * 0.03f + t;
                float ny = y * 0.03f + t * 0.5f;
                float v = noise2d(nx, ny);
                float v2 = noise2d(nx * 2.0f, ny * 2.0f) * 0.5f;
                v = (v + v2) / 1.5f; // octave sayısını 3'ten 2'ye indirdik (hız için)
                v = (v + 1.0f) * 0.5f;  // 0..1

                // Renkli gradient palet
                uint8_t r = (uint8_t)(v * 120 + 30);
                uint8_t g = (uint8_t)(v * 200 + 40);
                uint8_t b = (uint8_t)(v * 160 + 80);
                uint16_t color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

                // 3x3 piksel bloğunu boya
                int py_start = y * scale;
                int px_start = x * scale;
                for (int dy = 0; dy < scale; dy++) {
                    int rowOffset = (py_start + dy) * width;
                    for (int dx = 0; dx < scale; dx++) {
                        buffer[rowOffset + (px_start + dx)] = color;
                    }
                }
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
