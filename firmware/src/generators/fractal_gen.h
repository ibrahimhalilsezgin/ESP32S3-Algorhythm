#pragma once
#include "base_generator.h"

class FractalGen : public BaseGenerator {
public:
    void generate(uint16_t* buffer, int width, int height, uint32_t frame_num) override {
        // Mandelbrot zoom animasyonu
        float zoom = 1.0f + frame_num * 0.02f;
        float cx = -0.7435f;
        float cy = 0.1314f;
        float scale = 3.0f / (zoom * width);

        for (int py = 0; py < height; py++) {
            for (int px = 0; px < width; px++) {
                float x0 = (px - width / 2.0f) * scale + cx;
                float y0 = (py - height / 2.0f) * scale + cy;
                float x = 0, y = 0;
                int iter = 0;
                int maxIter = 32;  // düşük tut, ESP32 hızı için
                while (x * x + y * y <= 4.0f && iter < maxIter) {
                    float tmp = x * x - y * y + x0;
                    y = 2.0f * x * y + y0;
                    x = tmp;
                    iter++;
                }
                if (iter == maxIter) {
                    buffer[py * width + px] = 0x0000;
                } else {
                    uint8_t c = (uint8_t)((iter * 255) / maxIter);
                    // Mavi-mor gradient
                    uint8_t r = c / 2;
                    uint8_t g = 0;
                    uint8_t b = c;
                    buffer[py * width + px] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                }
            }
        }
    }
    const char* name() override { return "fractal"; }
};
