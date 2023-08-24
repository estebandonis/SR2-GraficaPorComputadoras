#pragma once
#include "color.h"
#include <glm.hpp>
#include <cstdint>

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
};

struct Fragment {
  uint16_t x;
  uint16_t y;
  double z;
  Color color;
  float intensitiy;
};

struct FragColor {
  Color color;
  double z;
};