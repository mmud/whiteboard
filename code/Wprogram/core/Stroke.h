#pragma once
#include <vector>
#include "Circle.h"

struct Stroke {
public:
    std::vector<Circle> circles;
    std::vector<float> color;
    float brushSize;

    Stroke(std::vector<Circle>& cir, std::vector<float>& col, float bsize)
        : circles(cir), brushSize(bsize)
    {
        color.resize(3);
        for (int i = 0; i < 3; i++) {
            color[i] = col[i];
        }
    }
};
