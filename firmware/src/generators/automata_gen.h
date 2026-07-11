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
        if (!_initialized || frame_num % 300 == 0) {
            init(width, height);
        }
        step(width, height);
        render(buffer, width, height);
    }
    const char* name() override { return "automata"; }

private:
    bool _initialized;
    uint8_t* _grid;
    uint8_t* _next;
    int _w, _h;

    void init(int w, int h) {
        _w = w;
        _h = h;
        size_t sz = w * h;
        if (!_grid) _grid = (uint8_t*)ps_malloc(sz);
        if (!_next) _next = (uint8_t*)ps_malloc(sz);
        // Rastgele başlat
        for (size_t i = 0; i < sz; i++) {
            _grid[i] = (rand() % 100) < 30 ? 1 : 0;  // %30 canlı
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
                        neighbors += _grid[ny * w + nx];
                    }
                }
                uint8_t cell = _grid[y * w + x];
                if (cell && (neighbors < 2 || neighbors > 3)) {
                    _next[y * w + x] = 0;
                } else if (!cell && neighbors == 3) {
                    _next[y * w + x] = 1;
                } else {
                    _next[y * w + x] = cell;
                }
            }
        }
        // Swap
        uint8_t* tmp = _grid;
        _grid = _next;
        _next = tmp;
    }

    void render(uint16_t* buffer, int w, int h) {
        for (int i = 0; i < w * h; i++) {
            buffer[i] = _grid[i] ? 0x07E0 : 0x0000;  // yeşil / siyah
        }
    }
};
