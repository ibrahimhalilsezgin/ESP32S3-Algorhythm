#pragma once
#include "base_generator.h"

class FractalGen : public BaseGenerator {
public:
    void generate(uint16_t* buffer, int width, int height, uint32_t frame_num) override {
        // Mandelbrot zoom animasyonu
        // 600x600 çözünürlükte performansı korumak için 2x2 bloklar halinde (300x300 mantıksal) çiziyoruz.
        int scale = 2;
        int logicalW = width / scale;
        int logicalH = height / scale;
        
        float zoom = 1.0f + (frame_num % 1000) * 0.05f; // Zoom döngüsü
        float cx = -0.7435f;
        float cy = 0.1314f;
        float scaleVal = 3.0f / (zoom * logicalW);

        for (int py = 0; py < logicalH; py++) {
            for (int px = 0; px < logicalW; px++) {
                float x0 = (px - logicalW / 2.0f) * scaleVal + cx;
                float y0 = (py - logicalH / 2.0f) * scaleVal + cy;
                float x = 0, y = 0;
                int iter = 0;
                int maxIter = 16;  // 600x600'de akıcı kalması için 16 yaptık
                while (x * x + y * y <= 4.0f && iter < maxIter) {
                    float tmp = x * x - y * y + x0;
                    y = 2.0f * x * y + y0;
                    x = tmp;
                    iter++;
                }
                
                uint16_t color;
                if (iter == maxIter) {
                    color = 0x0000;
                } else {
                    uint8_t c = (uint8_t)((iter * 255) / maxIter);
                    // Mavi-mor gradient
                    uint8_t r = c / 2;
                    uint8_t g = 0;
                    uint8_t b = c;
                    color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                }

                // 2x2 piksel bloğunu boya
                int py_start = py * scale;
                int px_start = px * scale;
                for (int dy = 0; dy < scale; dy++) {
                    int rowOffset = (py_start + dy) * width;
                    for (int dx = 0; dx < scale; dx++) {
                        buffer[rowOffset + (px_start + dx)] = color;
                    }
                }
            }
        }
    }
    const char* name() override { return "fractal"; }
};
