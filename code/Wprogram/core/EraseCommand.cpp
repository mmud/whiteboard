#include "EraseCommand.h"

EraseCommand::EraseCommand(const std::vector<int>& indices, std::vector<Stroke>* strokesRef)
    : strokeIndicesToRemove(indices), strokes(strokesRef), wasExecuted(false)
{
    std::sort(strokeIndicesToRemove.begin(), strokeIndicesToRemove.end(), std::greater<int>());
}

void EraseCommand::execute() {
    if (wasExecuted) return;

    removedStrokes.clear();

    for (int index : strokeIndicesToRemove) {
        if (index >= 0 && index < strokes->size()) {
            removedStrokes.push_back((*strokes)[index]);

            strokes->erase(strokes->begin() + index);
        }
    }

    wasExecuted = true;
}

void EraseCommand::undo() {
    if (!wasExecuted) return;

    for (int i = removedStrokes.size() - 1; i >= 0; i--) {
        int originalIndex = strokeIndicesToRemove[removedStrokes.size() - 1 - i];

        if (originalIndex <= strokes->size()) {
            strokes->insert(strokes->begin() + originalIndex, removedStrokes[i]);
        }
        else {
            strokes->push_back(removedStrokes[i]);
        }
    }

    wasExecuted = false;
}