#pragma once
#include "Command.h"
#include "Stroke.h"
#include "Circle.h"
#include <vector>

class DrawCommand : public Command {
private:
    std::vector<Circle> circles;
    std::vector<float> color;
    float brushSize;
    int strokeIndex;
    std::vector<Stroke>* strokes;

public:
    DrawCommand(std::vector<Circle>& cir, std::vector<float>& col, float size, std::vector<Stroke>* stRef);

    void execute() override;

    void undo() override;
};