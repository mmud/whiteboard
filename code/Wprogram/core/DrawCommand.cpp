#include "DrawCommand.h"

DrawCommand::DrawCommand(std::vector<Circle>& cir, std::vector<float>& col, float size, std::vector<Stroke>* stRef)
    : circles(cir), brushSize(size), strokes(stRef), strokeIndex(-1)
{
    color.resize(3);
    for (int i = 0; i < 3; i++) {
        color[i] = col[i];
    }
}

void DrawCommand::execute() {
    Stroke newStroke(circles, color, brushSize);
    strokes->push_back(newStroke);
    strokeIndex = strokes->size() - 1;
}

void DrawCommand::undo() {
    if (strokeIndex >= 0 && strokeIndex < strokes->size()) 
        strokes->erase(strokes->begin() + strokeIndex);
}