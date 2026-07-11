#pragma once
#include "base_generator.h"
#include <stdlib.h>

class PixelArtGen : public BaseGenerator {
public:
    void generate(uint16_t* buffer, int width, int height, uint32_t frame_num) override {
        // Her 60 frame'de yeni desen
        if (frame_num % 60 == 0) {
            _seed = (uint32_t)rand();
        }

        int blockSize = 8;
        int gridW = width / blockSize;
        int gridH = height / blockSize;
        int halfW = gridW / 2;

        srand(_seed);

        // Palet oluştur (3-5 renk)
        uint16_t palette[5];
        int palSize = 3 + (rand() % 3);
        for (int i = 0; i < palSize; i++) {
            uint8_t r = rand() % 256;
            uint8_t g = rand() % 256;
            uint8_t b = rand() % 256;
            palette[i] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }

        // Arka plan temizle
        uint16_t bg = palette[0];
        for (int i = 0; i < width * height; i++) buffer[i] = bg;

        // Simetrik desen çiz
        for (int gy = 0; gy < gridH; gy++) {
            for (int gx = 0; gx <= halfW; gx++) {
                if (rand() % 3 == 0) continue;  // boşluk
                uint16_t color = palette[1 + rand() % (palSize - 1)];
                fillBlock(buffer, width, gx * blockSize, gy * blockSize, blockSize, color);
                // Yatay ayna
                int mirrorX = (gridW - 1 - gx) * blockSize;
                fillBlock(buffer, width, mirrorX, gy * blockSize, blockSize, color);
            }
        }
    }
    const char* name() override { return "pixel_art"; }

private:
    uint32_t _seed = 0;

    void fillBlock(uint16_t* buffer, int stride, int x, int y, int size, uint16_t color) {
        for (int dy = 0; dy < size; dy++) {
            for (int dx = 0; dx < size; dx++) {
                buffer[(y + dy) * stride + (x + dx)] = color;
            }
        }
    }
};
