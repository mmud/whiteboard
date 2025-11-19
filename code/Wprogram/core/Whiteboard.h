#pragma once
#include <vector>
#include "Stroke.h"
#include "Circle.h"
#include "DrawCommand.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Renderer.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <stb_image_write.h>

class Whiteboard {
private:
    std::vector<Stroke> strokes;

    VertexArray* va;
    VertexBuffer* vb;
    IndexBuffer* ib;
    Shader* shader;
    Renderer* renderer;

    glm::mat4 proj;
    glm::mat4 view;

    std::vector<float> currentColor;
    float currentBrushSize;
    bool isDrawing;
    std::vector<Circle> tempCircles;

public:
    Whiteboard(int width, int height);
        
    ~Whiteboard();

    void startDrawing(float x, float y);

    void addCircle(float x, float y);

    DrawCommand* endDrawing();

    void render();

    void clear();

    void setColor(float r, float g, float b);

    void setBrushSize(float size);

    float getBrushSize() {return currentBrushSize;}

    std::vector<Stroke>& getStrokes() {return strokes;}

    bool getIsDrawing() {return isDrawing;}

    void updateProjection(int width, int height);

    bool saveDrawing(const std::string& filename, int sidebarWidth, int windowWidth, int windowHeight);
};