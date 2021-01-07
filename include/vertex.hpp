#pragma once
// vertex object
#include <external.hpp>
#include <fstream>

struct Vertex {
  glm::vec2 pos;
  glm::vec3 color;

  static VkVertexInputBindingDescription
  getBindingDescription() {
    VkVertexInputBindingDescription description{};
    description.binding = 0;
    description.stride = sizeof(Vertex);
    description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return description;
  }
  static std::array<VkVertexInputAttributeDescription, 2>
  getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 2>
        attributes{};
    attributes[0].binding = 0;
    attributes[0].location = 0;
    attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributes[0].offset = offsetof(Vertex, pos);
    //
    attributes[1].binding = 0;
    attributes[1].location = 1;
    attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributes[1].offset = offsetof(Vertex, color);
    return attributes;
  }
};

inline std::ostream &operator<<(std::ostream &out,
                                const Vertex &v) {
  return out << "vertex position: x: " << v.pos.x
             << " y: " << v.pos.y
             << " vertex color: r: " << v.color.x
             << " g: " << v.color.y << " b: " << v.color.z;
}
