#include <glad/glad.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <stack>
#include <fstream>
#include <string>

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

Whiteboard::DrawingMode g_currentMode = Whiteboard::DrawingMode::DRAW;

void SetupModernStyle();

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
                Command* cmd = g_whiteboard->endDrawing();

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

            if (g_whiteboard->getDrawingMode() == Whiteboard::DrawingMode::DRAW) {
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
            else
            {
                g_whiteboard->addCircle(worldPos.x, worldPos.y);

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
        SetupModernStyle();

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

            ImGui::Text("TOOLBAR");
            ImGui::Separator();
            ImGui::Spacing();

            ImVec2 btnSize(50, 30);

            ImGuiStyle& style = ImGui::GetStyle();
            float oldFrameBorder = style.FrameBorderSize;
            style.FrameBorderSize = 2.0f;  

            ImVec4 drawButtonColor = (g_currentMode == Whiteboard::DrawingMode::DRAW)
                ? ImVec4(0.2f, 0.7f, 0.3f, 1.0f)
                : ImVec4(0.20f, 0.20f, 0.20f, 1.0f);

            ImVec4 drawBorderColor = (g_currentMode == Whiteboard::DrawingMode::DRAW)
                ? ImVec4(0.3f, 1.0f, 0.4f, 1.0f) 
                : ImVec4(0.3f, 0.3f, 0.3f, 1.0f);

            ImGui::PushStyleColor(ImGuiCol_Button, drawButtonColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(drawButtonColor.x * 1.2f, drawButtonColor.y * 1.2f, drawButtonColor.z * 1.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(drawButtonColor.x * 0.8f, drawButtonColor.y * 0.8f, drawButtonColor.z * 0.8f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Border, drawBorderColor);

            if (ImGui::Button("DRAW", btnSize)) {
                whiteboard.setDrawingMode(Whiteboard::DrawingMode::DRAW);
                g_currentMode = Whiteboard::DrawingMode::DRAW;
            }
            ImGui::PopStyleColor(4);

            ImGui::SameLine();

            ImVec4 eraseButtonColor = (g_currentMode == Whiteboard::DrawingMode::ERASE)
                ? ImVec4(0.9f, 0.3f, 0.2f, 1.0f)
                : ImVec4(0.20f, 0.20f, 0.20f, 1.0f);

            ImVec4 eraseBorderColor = (g_currentMode == Whiteboard::DrawingMode::ERASE)
                ? ImVec4(1.0f, 0.4f, 0.3f, 1.0f) 
                : ImVec4(0.3f, 0.3f, 0.3f, 1.0f);

            ImGui::PushStyleColor(ImGuiCol_Button, eraseButtonColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(eraseButtonColor.x * 1.2f, eraseButtonColor.y * 1.2f, eraseButtonColor.z * 1.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(eraseButtonColor.x * 0.8f, eraseButtonColor.y * 0.8f, eraseButtonColor.z * 0.8f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Border, eraseBorderColor);

            if (ImGui::Button("ERASE", btnSize)) {
                whiteboard.setDrawingMode(Whiteboard::DrawingMode::ERASE);
                g_currentMode = Whiteboard::DrawingMode::ERASE;
            }
            ImGui::PopStyleColor(4);

            style.FrameBorderSize = oldFrameBorder;

            ImGui::Spacing();

            ImGui::Text("Brush Color");
            ImGui::Separator();

            ImGui::ColorPicker3("##colorpicker", brushColor,
                ImGuiColorEditFlags_NoSidePreview |
                ImGuiColorEditFlags_NoSmallPreview);

            ImGui::SliderFloat("Size", &brushSize, 0.15f, 0.5f);

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


            //ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

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


void SetupModernStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // === ROUNDED CORNERS ===
    style.WindowRounding = 12.0f;
    style.ChildRounding = 12.0f;
    style.FrameRounding = 8.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 10.0f;
    style.GrabRounding = 8.0f;
    style.TabRounding = 8.0f;

    // === PADDING & SPACING ===
    style.WindowPadding = ImVec2(15, 15);
    style.FramePadding = ImVec2(10, 6);
    style.ItemSpacing = ImVec2(12, 8);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 15.0f;
    style.GrabMinSize = 12.0f;

    // === BORDERS ===
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;

    // === MODERN DARK COLORS ===
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
    colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.08f, 0.50f, 0.72f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}