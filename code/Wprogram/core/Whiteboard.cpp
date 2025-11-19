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

bool Whiteboard::saveDrawing(const std::string& filename,int sidebarWidth,int windowWidth,int windowHeight) {
    int drawingWidth = windowWidth - sidebarWidth;
    int drawingHeight = windowHeight;

    unsigned char* pixels = new unsigned char[drawingWidth * drawingHeight * 4];

    glReadPixels(
        sidebarWidth,      // x: skip sidebar
        0,                 // y: from bottom
        drawingWidth,      // width: drawing area only
        drawingHeight,     // height: full height
        GL_RGBA,           // format: Red, Green, Blue, Alpha
        GL_UNSIGNED_BYTE,  // type: 8-bit unsigned byte per channel
        pixels             // destination buffer
    );

    // Flip image vertically (OpenGL is bottom-to-top, images are top-to-bottom)
    unsigned char* flippedPixels = new unsigned char[drawingWidth * drawingHeight * 4];
    for (int y = 0; y < drawingHeight; y++) {
        memcpy(
            flippedPixels + (drawingHeight - 1 - y) * drawingWidth * 4,
            pixels + y * drawingWidth * 4,
            drawingWidth * 4
        );
    }

    // Get file extension
    std::string ext = "";
    size_t dotPos = filename.find_last_of(".");
    if (dotPos != std::string::npos) {
        ext = filename.substr(dotPos + 1);
        // Convert to lowercase
        for (char& c : ext) {
            c = tolower(c);
        }
    }

    int result = 0;

    if (ext == "png") {
        result = stbi_write_png(
            filename.c_str(),        // filename
            drawingWidth,            // width
            drawingHeight,           // height
            4,                       // channels (RGBA)
            flippedPixels,          // pixel data
            drawingWidth * 4        // stride (bytes per row)
        );
    }
    else if (ext == "jpg" || ext == "jpeg") {
        result = stbi_write_jpg(
            filename.c_str(),        // filename
            drawingWidth,            // width
            drawingHeight,           // height
            4,                       // channels (RGBA)
            flippedPixels,          // pixel data
            95                      // quality (1-100, 95 is high quality)
        );
    }
    else if (ext == "bmp") {
        result = stbi_write_bmp(
            filename.c_str(),        // filename
            drawingWidth,            // width
            drawingHeight,           // height
            4,                       // channels (RGBA)
            flippedPixels           // pixel data
        );
    }
    else {
        std::string pngFilename = filename + ".png";
        result = stbi_write_png(
            pngFilename.c_str(),
            drawingWidth,
            drawingHeight,
            4,
            flippedPixels,
            drawingWidth * 4
        );
    }

    delete[] pixels;
    delete[] flippedPixels;

    return result != 0;
}