cmake_minimum_required(VERSION 3.23)

project(SR VERSION 1.0)

# Enable C++20 features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Specify the path to the glm library headers
set(GLM_INCLUDE_DIR /Users/estebandonis/Documents/SextoSemestre/GraficasPorComputadora/include/glm)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_executable(${PROJECT_NAME}
    src/main.cpp
    ${SOURCES}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE ${PROJECT_SOURCE_DIR}/include
    PUBLIC ${PROJECT_SOURCE_DIR}/src
    PRIVATE ${GLM_INCLUDE_DIR}  # Add the glm library path here
)

target_link_libraries(${PROJECT_NAME}
    ${SDL2_LIBRARIES}
)
