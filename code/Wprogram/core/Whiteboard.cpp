#include "Whiteboard.h"

Whiteboard::Whiteboard(int width, int height) {
	currentColor.resize(3);

    float positions[] = {
        // positions      // texture coords
        -0.5f, -0.5f,     0.0f, 0.0f,  // Bottom-left
         0.5f, -0.5f,     1.0f, 0.0f,  // Bottom-right
         0.5f,  0.5f,     1.0f, 1.0f,  // Top-right
        -0.5f,  0.5f,     0.0f, 1.0f   // Top-left
    };
    unsigned int indices[] = {
           0, 1, 2,
           2, 3, 0
    };

    va = new VertexArray();
    vb = new VertexBuffer(positions, sizeof(positions));
    ib = new IndexBuffer(indices, 6);
    VertexBufferLayout layout;
    layout.Push<float>(2);
    layout.Push<float>(2);
    va->AddBuffer(*vb, layout);
    shader=new Shader(std::string(SHADER_PATH) + "/Basic.shader");
    renderer = &Renderer::getInstance();

    float aspect = (float)width / (float)height;
    proj = glm::ortho(
        -2.0f * aspect, 2.0f * aspect,  // Left, Right
        -2.0f, 2.0f,                     // Bottom, Top
        -1.0f, 1.0f                      // Near, Far
    );
    view = glm::mat4(1.0f);

    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    currentColor[0] = 0.0f;
    currentColor[1] = 0.0f;
    currentColor[2] = 0.0f;

    currentBrushSize = 0.05f;

    isDrawing = false;
}

Whiteboard::~Whiteboard() {
    delete va;
    delete vb;
    delete ib;
    delete shader;
}

void Whiteboard::startDrawing(float x, float y) {
    isDrawing = true;

    tempCircles.clear();

    Circle firstCircle(x, y, currentBrushSize);
    tempCircles.push_back(firstCircle);
}

void Whiteboard::addCircle(float x, float y) {
    if (!isDrawing) return;

    Circle newCircle(x, y, currentBrushSize);
    tempCircles.push_back(newCircle);
}

DrawCommand* Whiteboard::endDrawing() {
    if (!isDrawing) return nullptr; 

    isDrawing = false;

    if (tempCircles.empty()) return nullptr;
    

    DrawCommand* cmd = new DrawCommand(
        tempCircles,
        currentColor,
        currentBrushSize,
        &strokes
    );

    return cmd;
}

void Whiteboard::render() {
    renderer->Clear();
    shader->Bind();

    for (int i = 0; i < strokes.size(); i++) {
        Stroke& stroke = strokes[i];

        for (int j = 0; j < stroke.circles.size(); j++) {
            Circle& circle = stroke.circles[j];

            glm::mat4 model = glm::mat4(1.0f);

            model = glm::translate(model, glm::vec3(
                circle.centerX,
                circle.centerY,
                0.0f
            ));

            float diameter = circle.raduis * 2.0f;
            model = glm::scale(model, glm::vec3(
                diameter,
                diameter,
                1.0f
            ));

            glm::mat4 mvp = proj * view * model;

            shader->SetUniformMat4f("u_MVP", mvp);

            shader->SetUniform4f("u_color",
                stroke.color[0],
                stroke.color[1],
                stroke.color[2],
                1.0f);

            shader->SetUniform2f("circleCenter", 0.5f, 0.5f);

            shader->SetUniform1f("circleRadius", stroke.brushSize);

            renderer->Draw(*va, *ib, *shader);
        }
    }

    if (isDrawing && !tempCircles.empty()) {
        for (int i = 0; i < tempCircles.size(); i++) {
            Circle& circle = tempCircles[i];

            glm::mat4 model = glm::mat4(1.0f);

            model = glm::translate(model, glm::vec3(
                circle.centerX,
                circle.centerY,
                0.0f
            ));

            float diameter = circle.raduis * 2.0f;
            model = glm::scale(model, glm::vec3(
                diameter,
                diameter,
                1.0f
            ));

            glm::mat4 mvp = proj * view * model;

            shader->SetUniformMat4f("u_MVP", mvp);

            shader->SetUniform4f("u_color",
                currentColor[0],
                currentColor[1],
                currentColor[2],
                1.0f);

            shader->SetUniform2f("circleCenter", 0.5f, 0.5f);
            shader->SetUniform1f("circleRadius", currentBrushSize);

            renderer->Draw(*va, *ib, *shader);
        }
    }
    
}

void Whiteboard::clear() {
    strokes.clear();
}

void Whiteboard::setColor(float r, float g, float b) {
    currentColor[0] = r;
    currentColor[1] = g;
    currentColor[2] = b;
}

void Whiteboard::setBrushSize(float size) {
    currentBrushSize = size;
}

void Whiteboard::updateProjection(int width, int height) {
    float aspect = (float)width / (float)height;
    proj = glm::ortho(
        -2.0f * aspect, 2.0f * aspect,
        -2.0f, 2.0f,
        -1.0f, 1.0f
    );
}