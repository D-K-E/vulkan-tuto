#pragma once
// vertex object
#include <external.hpp>

struct Vertex {
  glm::vec2 pos;
  glm::vec3 color;
  glm::vec2 texCoord;

  static VkVertexInputBindingDescription
  getBindingDescription() {
    VkVertexInputBindingDescription description{};
    description.binding = 0;
    description.stride = sizeof(Vertex);
    description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return description;
  }
  static std::array<VkVertexInputAttributeDescription, 3>
  getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 3>
        attributes{};

    // set position
    attributes[0].binding = 0;
    attributes[0].location = 0;
    attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributes[0].offset = offsetof(Vertex, pos);

    // color
    attributes[1].binding = 0;
    attributes[1].location = 1;
    attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributes[1].offset = offsetof(Vertex, color);

    // texture coordinates
    attributes[2].binding = 0;
    attributes[2].location = 2;
    attributes[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributes[2].offset = offsetof(Vertex, texCoord);

    return attributes;
  }
};

inline std::ostream &operator<<(std::ostream &out,
                                const Vertex &v) {
  return out << "vertex position: x: " << v.pos.x
             << " y: " << v.pos.y << std::endl
             << " vertex color: r: " << v.color.x
             << " g: " << v.color.y << " b: " << v.color.z
             << " vertex texCoord x" << v.texCoord.x
             << " y: " << v.texCoord.y << std::endl;
}
