#pragma once

#include "Command.h"
#include "Stroke.h"
#include "Circle.h"
#include <vector>
#include <algorithm>

class EraseCommand : public Command {
private:
    std::vector<int> strokeIndicesToRemove;
    std::vector<Stroke> removedStrokes;
    std::vector<Stroke>* strokes;
    bool wasExecuted;

public:
    EraseCommand(const std::vector<int>& indices, std::vector<Stroke>* strokesRef);

    void execute() override;

    void undo() override;

};