#include <glad/glad.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <stack>

#include "Renderer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "Whiteboard.h"
#include "Command.h"
#include "DrawCommand.h"

Whiteboard* g_whiteboard = nullptr;
std::stack<Command*>* g_undoStack = nullptr;
std::stack<Command*>* g_redoStack = nullptr;
double g_lastMouseX = 0.0;
double g_lastMouseY = 0.0;
const float SIDEBAR_WIDTH = 300.0f;

#ifdef _WIN32
std::string openSaveFileDialog() {
    char filename[MAX_PATH] = "whiteboard.png";

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "PNG Image (*.png)\0*.png\0"
        "JPEG Image (*.jpg)\0*.jpg;*.jpeg\0"
        "BMP Image (*.bmp)\0*.bmp\0"
        "All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
    ofn.lpstrDefExt = "png";
    ofn.lpstrTitle = "Save Whiteboard Drawing";

    if (GetSaveFileNameA(&ofn)) {
        return std::string(filename);
    }

    return "";
}
#endif

glm::vec2 screenToWorld(double screenX, double screenY,
    int windowWidth, int windowHeight) {
    // Adjust for sidebar
    float adjustedWidth = windowWidth - SIDEBAR_WIDTH;
    float adjustedX = screenX - SIDEBAR_WIDTH;

    // Calculate aspect ratio
    float aspect = adjustedWidth / (float)windowHeight;

    // Normalize screen coordinates (0 to 1)
    float normalizedX = adjustedX / adjustedWidth;
    float normalizedY = screenY / windowHeight;

    // Convert to world coordinates (-2*aspect to 2*aspect, -2 to 2)
    float worldX = normalizedX * (4.0f * aspect) - (2.0f * aspect);
    float worldY = 2.0f - normalizedY * 4.0f;

    return glm::vec2(worldX, worldY);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            if (xpos >= SIDEBAR_WIDTH) {
                int width, height;
                glfwGetFramebufferSize(window, &width, &height);

                glm::vec2 worldPos = screenToWorld(xpos, ypos, width, height);

                g_whiteboard->startDrawing(worldPos.x, worldPos.y);

                g_lastMouseX = worldPos.x;
                g_lastMouseY = worldPos.y;
            }
        }
        else if (action == GLFW_RELEASE) {
            if (g_whiteboard->getIsDrawing()) {
                DrawCommand* cmd = g_whiteboard->endDrawing();

                if (cmd != nullptr) {
                    cmd->execute();

                    g_undoStack->push(cmd);

                    while (!g_redoStack->empty()) {
                        delete g_redoStack->top();
                        g_redoStack->pop();
                    }
                }
            }
        }
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS &&
        g_whiteboard->getIsDrawing()) {

        if (xpos >= SIDEBAR_WIDTH) {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            glm::vec2 worldPos = screenToWorld(xpos, ypos, width, height);

            float dx = worldPos.x - g_lastMouseX;
            float dy = worldPos.y - g_lastMouseY;
            float distance = sqrt(dx * dx + dy * dy);

            if (distance > 0.01f) {
                g_whiteboard->addCircle(worldPos.x, worldPos.y);

                if (distance > 0.05) {
                    int numSteps = (int)(distance / 0.05) + 1;

                    for (int i = 1; i <= numSteps; i++) {
                        float t = (float)i / (float)numSteps;

                        float interpX = g_lastMouseX + dx * t;
                        float interpY = g_lastMouseY + dy * t;

                        g_whiteboard->addCircle(interpX, interpY);
                    }
                }

                g_lastMouseX = worldPos.x;
                g_lastMouseY = worldPos.y;
            }
        }
    }
}


int main()
{
	 GLFWwindow* window;
    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1280 , 600, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }


    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    gladLoadGL();

    {//to fix the program terminate
        /*float positoins[] = {
            -0.5f, -0.5f, 0.0f, 0.0f,
             0.5f, -0.5f, 1.0f, 0.0f,
             0.5f,  0.5f, 1.0f, 1.0f,
            -0.5f,  0.5f, 0.0f, 1.0f
        };

        unsigned int indices[] = {
            0,1,2,
            2,3,0
        };

        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        VertexArray va;
        VertexBuffer vb(positoins, sizeof(positoins));

        VertexBufferLayout layout;
        layout.Push<float>(2);
        layout.Push<float>(2);
        va.AddBuffer(vb, layout);

        IndexBuffer ib(indices, 6);

        glm::mat4 proj = glm::ortho(-2.0f, 2.0f, -1.5f, 1.5f, -1.0f, 1.0f);
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));

        glm::mat4 mvp = proj * view * model;


        Shader shader(std::string(SHADER_PATH) + "/Basic.shader");
        std::cout << "shader"<<std::endl;
        shader.Bind();
        shader.SetUniform4f("u_color", 1.0f, 0.0f, 0.0f, 1.0f);
        shader.SetUniformMat4f("u_MVP", mvp);
        shader.SetUniform2f("circleCenter", 0.5, 0.5);
        shader.SetUniform1f("circleRadius", 0.5);
        */

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        Whiteboard whiteboard(width - SIDEBAR_WIDTH, height);

        std::stack<Command*> undoStack;
        std::stack<Command*> redoStack;


        g_whiteboard = &whiteboard;
        g_undoStack = &undoStack;
        g_redoStack = &redoStack;


        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetCursorPosCallback(window, cursorPosCallback);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; 

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");

        float brushColor[3] = { 0.2f, 0.3f, 0.4f };
        float brushSize = 0.3f;

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);

            // Undo (Ctrl+Z)
            if (ImGui::IsKeyPressed(ImGuiKey_Z) &&
                (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) ||
                    ImGui::IsKeyDown(ImGuiKey_RightCtrl))) {

                if (!undoStack.empty()) {
                    Command* cmd = undoStack.top();
                    undoStack.pop();

                    cmd->undo();

                    redoStack.push(cmd);
                }
            }

            // Redo (Ctrl+Y or Ctrl+Shift+Z)
            if ((ImGui::IsKeyPressed(ImGuiKey_Y) &&
                (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) ||
                    ImGui::IsKeyDown(ImGuiKey_RightCtrl))) ||
                (ImGui::IsKeyPressed(ImGuiKey_Z) &&
                    (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) ||
                        ImGui::IsKeyDown(ImGuiKey_RightCtrl)) &&
                    (ImGui::IsKeyDown(ImGuiKey_LeftShift) ||
                        ImGui::IsKeyDown(ImGuiKey_RightShift)))) {

                if (!redoStack.empty()) {
                    Command* cmd = redoStack.top();
                    redoStack.pop();

                    cmd->execute();

                    undoStack.push(cmd);
                }
            }


            glViewport(SIDEBAR_WIDTH, 0, display_w - SIDEBAR_WIDTH, display_h);

            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

            g_whiteboard->setColor(brushColor[0], brushColor[1], brushColor[2]);
            g_whiteboard->setBrushSize(brushSize);
            g_whiteboard->render();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(SIDEBAR_WIDTH, (float)display_h));
            ImGui::Begin("Sidebar", nullptr,
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse);

            ImGui::Text("Controls");
            ImGui::Separator();

            ImGui::ColorEdit3("circle Color", brushColor);
            ImGui::SliderFloat("circle Radius", &brushSize, 0.15f, 0.5f);

            if (ImGui::Button("Clear")) {
                whiteboard.clear();

                while (!undoStack.empty()) {
                    delete undoStack.top();
                    undoStack.pop();
                }
                while (!redoStack.empty()) {
                    delete redoStack.top();
                    redoStack.pop();
                }
            }


            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);


            /*88888888888888888888888888888888888888888888888888888888888888888*/

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

#ifdef _WIN32
            if (ImGui::Button("Save as", ImVec2(-1, 45))) {
                std::string filename = openSaveFileDialog();

                if (!filename.empty()) {
                    int display_w, display_h;
                    glfwGetFramebufferSize(window, &display_w, &display_h);

                    bool success = whiteboard.saveDrawing(filename, SIDEBAR_WIDTH, display_w, display_h);

                    if (!success)
                        std::cerr << "Failed to save the image" << std::endl;
                }
            }
#endif
        

        ImGui::Spacing();

        /*8888888888888888888888888888888888888888888888888888888888888888*/



            ImGui::End();

            ImGui::Render();

            glViewport(0, 0, display_w, display_h);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            /* Swap front and back buffers */
            glfwSwapBuffers(window);
        }
        // Cleanup
        while (!undoStack.empty()) {
            delete undoStack.top();
            undoStack.pop();
        }

        while (!redoStack.empty()) {
            delete redoStack.top();
            redoStack.pop();
        }
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    
    }
    glfwTerminate();
    return 0;
}