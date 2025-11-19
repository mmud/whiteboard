target_include_directories(${PROJECT_NAME} PRIVATE ..)
target_link_libraries(${PROJECT_NAME} PRIVATE GraphicsEngine)
target_sources(${PROJECT_NAME} PRIVATE main.cpp "main.cpp"  "Circle.h" "Stroke.h" "DrawCommand.h" "Whiteboard.h" "Whiteboard.cpp" "DrawCommand.cpp")
