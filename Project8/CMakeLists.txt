cmake_minimum_required(VERSION 3.10)
project(GeometryDash)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(GLUT REQUIRED)

add_executable(Project8
    main.cpp
    renderer.cpp
    player.cpp
    obstacle.cpp
    level.cpp
)

target_link_libraries(Project8
    OpenGL::OpenGL
    GLEW::GLEW
    glm::glm
    GLUT::GLUT
)

target_include_directories(Project8 PRIVATE
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
    ${GLM_INCLUDE_DIR}
    ${GLUT_INCLUDE_DIR}
)