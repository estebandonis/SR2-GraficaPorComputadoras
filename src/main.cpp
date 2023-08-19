#include <SDL2/SDL.h>
#include <vector>
#include <glm.hpp>
#include <gtx/io.hpp>
#include <gtc/matrix_transform.hpp>
#include <array>
#include <fstream>
#include <sstream>
#include <string>

#include "fragment.h"
#include "uniform.h"

#include "print.h"
#include "color.h"
#include "shaders.h"
#include "triangle.h"

const int WINDOW_WIDTH = 500;
const int WINDOW_HEIGHT = 500;

std::array<std::array<float, WINDOW_WIDTH>, WINDOW_HEIGHT> zbuffer;


SDL_Renderer* renderer;

Uniform uniform;


// Declare a global clearColor of type Color
Color clearColor = {0, 0, 0}; // Initially set to black

// Declare a global currentColor of type Color
Color currentColor = {255, 255, 255}; // Initially set to white

// Function to clear the framebuffer with the clearColor
void clear() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);


    for (auto &row : zbuffer) {
        std::fill(row.begin(), row.end(), 99999.0f);
    }
}

// Function to set a specific pixel in the framebuffer to the currentColor
void point(Fragment f) {
    if (f.position.z < zbuffer[f.position.y][f.position.x]) {
        SDL_SetRenderDrawColor(renderer, f.color.r, f.color.g, f.color.b, f.color.a);
        SDL_RenderDrawPoint(renderer, f.position.x, f.position.y);
        zbuffer[f.position.y][f.position.x] = f.position.z;
    }
}

std::vector<std::vector<Vertex>> primitiveAssembly(
    const std::vector<Vertex>& transformedVertices
) {
    std::vector<std::vector<Vertex>> groupedVertices;

    for (int i = 0; i < transformedVertices.size(); i += 3) {
        std::vector<Vertex> vertexGroup;
        vertexGroup.push_back(transformedVertices[i]);
        vertexGroup.push_back(transformedVertices[i+1]);
        vertexGroup.push_back(transformedVertices[i+2]);
        
        groupedVertices.push_back(vertexGroup);
    }

    return groupedVertices;
}


void render(std::vector<glm::vec3> VBO) {
    // 1. Vertex Shader
    // vertex -> trasnformedVertices
    
    std::vector<Vertex> transformedVertices;

    for (int i = 0; i < VBO.size(); i+=2) {
        glm::vec3 v = VBO[i];
        glm::vec3 c = VBO[i+1];

        Vertex vertex = {v, Color(c.x, c.y, c.z)};
        Vertex transformedVertex = vertexShader(vertex, uniform);
        transformedVertices.push_back(transformedVertex);
    }


    // 2. Primitive Assembly
    // transformedVertices -> triangles
    std::vector<std::vector<Vertex>> triangles = primitiveAssembly(transformedVertices);

    // 3. Rasterize
    // triangles -> Fragments
    std::vector<Fragment> fragments;
    for (const std::vector<Vertex>& triangleVertices : triangles) {
        std::vector<Fragment> rasterizedTriangle = triangle(
            triangleVertices[0],
            triangleVertices[1],
            triangleVertices[2]
        );
        
        fragments.insert(
            fragments.end(),
            rasterizedTriangle.begin(),
            rasterizedTriangle.end()
        );
    }

    // 4. Fragment Shader
    // Fragments -> colors

    for (Fragment fragment : fragments) {
        point(fragmentShader(fragment));
    }
}


float a = 3.14f / 3.0f;

glm::mat4 createModelMatrix() {
    glm::mat4 transtation = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(1.0f, 1.0f, 1.0f));
    glm::mat4 rotation = glm::rotate(glm::mat4(1), glm::radians(a++), glm::vec3(0.0f, 1.0f, 0.0f));
    
    return transtation * scale * rotation;
}

glm::mat4 createViewMatrix() {
    return glm::lookAt(
        // donde esta
        glm::vec3(0, 0, -5),
        // hacia adonde mira
        glm::vec3(0, 0, 0),
        // arriba
        glm::vec3(0, -20, 0)
    );
}

glm::mat4 createProjectionMatrix() {
  float fovInDegrees = 45.0f;
  float aspectRatio = WINDOW_WIDTH / WINDOW_HEIGHT;
  float nearClip = 0.1f;
  float farClip = 100.0f;

  return glm::perspective(glm::radians(fovInDegrees), aspectRatio, nearClip, farClip);
}

glm::mat4 createViewportMatrix() {
    glm::mat4 viewport = glm::mat4(1.0f);

    // Scale
    viewport = glm::scale(viewport, glm::vec3(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f, 0.5f));

    // Translate
    viewport = glm::translate(viewport, glm::vec3(1.0f, 1.0f, 0.5f));

    return viewport;
}

struct Face {
    std::vector<std::array<int, 3>> vertexIndices;
};

bool loadOBJ(const std::string& path, std::vector<glm::vec3>& out_vertices, std::vector<glm::vec3>& out_textures, std::vector<Face>& out_faces, float scaleFactor = 1.0f)
{
    std::ifstream file(path);
    if (!file)
    {
        std::cout << "Failed to open the file: " << path << std::endl;
        return false;
    }

    std::string line;
    std::istringstream iss;
    std::string lineHeader;
    glm::vec3 vertex;
    Face face;

    while (std::getline(file, line))
    {
        iss.clear();
        iss.str(line);
        iss >> lineHeader;

        if (lineHeader == "v")
        {
            iss >> vertex.x >> vertex.y >> vertex.z;
            vertex *= scaleFactor;
            out_vertices.push_back(vertex);
        }
        else if (lineHeader == "vt")
        {
            iss >> vertex.x >> vertex.y >> vertex.z;
            out_textures.push_back(vertex);
        }
        else if (lineHeader == "f")
        {
            std::array<int, 3> vertexIndices;
            while (iss >> lineHeader)
            {
              std::istringstream tokenstream(lineHeader);
              std::string token;
              std::array<int, 3> vertexIndices;

              // Read all three values separated by '/'
              for (int i = 0; i < 3; ++i) {
                  std::getline(tokenstream, token, '/');
                  vertexIndices[i] = std::stoi(token) - 1;
              }

              face.vertexIndices.push_back(vertexIndices);
            }
            out_faces.push_back(face);
            face.vertexIndices.clear();
        }
    }

    return true;
}

// Function that prints the contents of a vector of glm::vec3
auto printVec3Vector = [](const std::vector<glm::vec3>& vec) {
    for (const auto& v : vec) {
        std::cout << v << std::endl;
    }
};


int main() {
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow(".obj Renderer", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0);

    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    bool running = true;
    SDL_Event event;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> textures;
    std::vector<Face> faces;
    std::vector<glm::vec3> vertexBufferObject;

    if (loadOBJ("assets/Nave.obj", vertices, textures, faces, 0.05f)) {
        // For each face
        for (const auto& face : faces)
        {
            // For each vertex in the face
            for (const auto& vertexIndices : face.vertexIndices)
            {
                // Get the vertex position and normal from the input arrays using the indices from the face
                glm::vec3 vertexPosition = vertices[vertexIndices[0]];
                glm::vec3 vertexColor = glm::vec3(0.5f, 0.5f, 0.5f);


                // Add the vertex position and normal to the vertex array
                vertexBufferObject.push_back(vertexPosition);
                vertexBufferObject.push_back(vertexColor);
            }
        }
    }

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event. type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                        break;
                    case SDLK_DOWN:
                        break;
                    case SDLK_LEFT:
                        break;
                    case SDLK_RIGHT:
                        break;
                    case SDLK_s:
                        break;
                    case SDLK_a:
                        break;
                }
            }
        }

        uniform.model = createModelMatrix();
        uniform.view = createViewMatrix();
        uniform.projection = createProjectionMatrix();
        uniform.viewport = createViewportMatrix();

        clear();

        // Call our render function
        render(vertexBufferObject);

        // Present the frame buffer to the screen
        SDL_RenderPresent(renderer);

        // Delay to limit the frame rate
        SDL_Delay(1000 / 60);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}