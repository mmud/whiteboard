project(Wprogram LANGUAGES CXX)

add_executable(Wprogram)

add_subdirectory(core)

add_subdirectory(opengl)

target_compile_definitions(Wprogram PRIVATE 
    SHADER_PATH="${CMAKE_SOURCE_DIR}/code/Wprogram/opengl/shaders"
    IMAGES_PATH="${CMAKE_SOURCE_DIR}/code/Wprogram/opengl/images"
)