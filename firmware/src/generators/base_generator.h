#pragma once
#include <stdint.h>

class BaseGenerator {
public:
    virtual ~BaseGenerator() {}
    // RGB565 buffer'a frame üret
    virtual void generate(uint16_t* buffer, int width, int height, uint32_t frame_num) = 0;
    virtual const char* name() = 0;
};
