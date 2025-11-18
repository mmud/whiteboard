set(OPENGL_SOURCES
    IndexBuffer.cpp
    Renderer.cpp
    Shader.cpp
    stb_image.cpp
    Texture.cpp
    VertexArray.cpp
    VertexBuffer.cpp
)

add_library(GraphicsEngine STATIC ${OPENGL_SOURCES})

target_include_directories(GraphicsEngine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        glm
)

target_link_libraries(GraphicsEngine
    PUBLIC
        glad
        glfw
        imgui
)