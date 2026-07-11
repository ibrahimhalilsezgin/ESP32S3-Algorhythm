#pragma once
#include "base_generator.h"
#include <string.h>
#include <stdlib.h>

class AutomataGen : public BaseGenerator {
public:
    AutomataGen() : _initialized(false), _grid(nullptr), _next(nullptr) {}
    ~AutomataGen() {
        if (_grid) free(_grid);
        if (_next) free(_next);
    }

    void generate(uint16_t* buffer, int width, int height, uint32_t frame_num) override {
        // 600x600 çözünürlük için mantıksal 150x150 grid kullanacağız (4x4 bloklar)
        int blockScale = 4;
        int logicalW = width / blockScale;
        int logicalH = height / blockScale;

        if (!_initialized || frame_num % 400 == 0) {
            init(logicalW, logicalH);
        }
        step(logicalW, logicalH);
        render(buffer, width, height, logicalW, logicalH, blockScale, frame_num);
    }
    const char* name() override { return "automata"; }

private:
    bool _initialized;
    uint8_t* _grid; // Hücrelerin yaşını tutacak (0 = ölü, 1..255 = canlı ve yaşlanıyor)
    uint8_t* _next;
    int _logicalW, _logicalH;

    void init(int w, int h) {
        _logicalW = w;
        _logicalH = h;
        size_t sz = w * h;
        if (!_grid) _grid = (uint8_t*)ps_malloc(sz);
        if (!_next) _next = (uint8_t*)ps_malloc(sz);
        
        // Rastgele başlat
        for (size_t i = 0; i < sz; i++) {
            _grid[i] = (rand() % 100) < 25 ? 1 : 0; // %25 başlangıç canlılığı
        }
        _initialized = true;
    }

    void step(int w, int h) {
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                int neighbors = 0;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dy == 0) continue;
                        int nx = (x + dx + w) % w;
                        int ny = (y + dy + h) % h;
                        neighbors += (_grid[ny * w + nx] > 0) ? 1 : 0;
                    }
                }
                
                uint8_t age = _grid[y * w + x];
                if (age > 0) {
                    if (neighbors < 2 || neighbors > 3) {
                        _next[y * w + x] = 0; // Aşırı nüfus veya yalnızlıktan ölüm
                    } else {
                        // Yaşamaya devam ediyor, yaşını artır (renk değişimi için)
                        _next[y * w + x] = (age < 255) ? age + 1 : 255;
                    }
                } else {
                    if (neighbors == 3) {
                        _next[y * w + x] = 1; // Yeni doğum
                    } else {
                        _next[y * w + x] = 0;
                    }
                }
            }
        }
        // Swap
        uint8_t* tmp = _grid;
        _grid = _next;
        _next = tmp;
    }

    void render(uint16_t* buffer, int width, int height, int lw, int lh, int scale, uint32_t frame_num) {
        // Her mantıksal hücreyi scale x scale boyutunda piksel bloğu olarak çiz
        for (int ly = 0; ly < lh; ly++) {
            for (int lx = 0; lx < lw; lx++) {
                uint8_t age = _grid[ly * lw + lx];
                uint16_t color = 0x0000; // Siyah arka plan

                if (age > 0) {
                    // Yaşa bağlı gökkuşağı geçişi
                    uint8_t r = (age * 3 + (frame_num / 2)) % 32;       // 5-bit R
                    uint8_t g = (age * 5 + 30) % 64;                    // 6-bit G
                    uint8_t b = (age * 2 + 10) % 32;                    // 5-bit B
                    
                    // Renklerin çok karanlık kalmaması için taban parlaklık ekleyelim
                    if (r < 8) r += 8;
                    if (g < 16) g += 16;
                    if (b < 8) b += 8;

                    color = (r << 11) | (g << 5) | b;
                }

                // Buffer'a scale x scale bloğu yaz
                int py_start = ly * scale;
                int px_start = lx * scale;
                for (int dy = 0; dy < scale; dy++) {
                    int rowOffset = (py_start + dy) * width;
                    for (int dx = 0; dx < scale; dx++) {
                        buffer[rowOffset + (px_start + dx)] = color;
                    }
                }
            }
        }
    }
};
