#include <glad/glad.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include<iostream>

#include "Renderer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

int main()
{
	 GLFWwindow* window;
    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
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
        float positoins[] = {
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

        Renderer renderer;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; 

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");

        float bgColor[3] = { 0.2f, 0.3f, 0.4f };
        float rd = 0.0f;

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);

            const float sidebar_width = 300.0f;

            glViewport(sidebar_width, 0, display_w - sidebar_width, display_h);

            renderer.Clear();

            shader.Bind();
            shader.SetUniform4f("u_color", bgColor[0], bgColor[1], bgColor[2], 1.0f);
            shader.SetUniform1f("circleRadius", rd);
            renderer.Draw(va, ib, shader);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(sidebar_width, (float)display_h));
            ImGui::Begin("Sidebar", nullptr,
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse);

            ImGui::Text("Controls");
            ImGui::Separator();

            ImGui::ColorEdit3("circle Color", bgColor);
            ImGui::SliderFloat("circle Radius", &rd, 0.0f, 0.5f);

            if (ImGui::Button("Reset")) {
                rd = 0.0f;
            }

            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

            ImGui::End();

            ImGui::Render();

            glViewport(0, 0, display_w, display_h);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            /* Swap front and back buffers */
            glfwSwapBuffers(window);
        }
        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    
    }
    glfwTerminate();
    return 0;
}