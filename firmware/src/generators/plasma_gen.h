#pragma once
#include "base_generator.h"
#include <math.h>

class PlasmaGen : public BaseGenerator {
public:
    void generate(uint16_t* buffer, int width, int height, uint32_t frame_num) override {
        float t = frame_num * 0.05f;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                float v = sinf(x * 0.04f + t);
                v += sinf(y * 0.03f + t * 0.7f);
                v += sinf((x + y) * 0.02f + t * 0.5f);
                v += sinf(sqrtf(x * x + y * y) * 0.03f + t * 0.3f);
                // v: -4..4 → 0..1
                v = (v + 4.0f) / 8.0f;
                // HSV→RGB basitleştirilmiş
                uint8_t r, g, b;
                hsvToRgb(v, 0.8f, 0.9f, r, g, b);
                buffer[y * width + x] = rgb565(r, g, b);
            }
        }
    }
    const char* name() override { return "plasma"; }

private:
    static uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }

    static void hsvToRgb(float h, float s, float v, uint8_t& r, uint8_t& g, uint8_t& b) {
        h = fmodf(h, 1.0f);
        if (h < 0) h += 1.0f;
        int i = (int)(h * 6.0f);
        float f = h * 6.0f - i;
        float p = v * (1.0f - s);
        float q = v * (1.0f - f * s);
        float t = v * (1.0f - (1.0f - f) * s);
        float rf, gf, bf;
        switch (i % 6) {
            case 0: rf = v; gf = t; bf = p; break;
            case 1: rf = q; gf = v; bf = p; break;
            case 2: rf = p; gf = v; bf = t; break;
            case 3: rf = p; gf = q; bf = v; break;
            case 4: rf = t; gf = p; bf = v; break;
            default: rf = v; gf = p; bf = q; break;
        }
        r = (uint8_t)(rf * 255);
        g = (uint8_t)(gf * 255);
        b = (uint8_t)(bf * 255);
    }
};
