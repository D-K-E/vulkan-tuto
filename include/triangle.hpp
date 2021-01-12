#pragma once
// triangle object
#include <external.hpp>
#include <vertex.hpp>

struct Triangle {
  Vertex p1;
  Vertex p2;
  Vertex p3;
  Triangle() {}
  Triangle(const Vertex &p1_, const Vertex &p2_,
           const Vertex &p3_)
      : p1(p1_), p2(p2_), p3(p3_) {}
  Triangle(const std::vector<Vertex> &vs) {
    if (vs.size() < 3) {
      throw std::runtime_error(
          "vector has less than 3 vertices");
    }
    p1 = vs[0];
    p2 = vs[1];
    p3 = vs[2];
  }
  Triangle(const std::array<Vertex, 3> &vs)
      : p1(vs[0]), p2(vs[1]), p3(vs[2]) {}

  std::vector<Vertex> to_vector() const {
    std::vector<Vertex> vs(3);
    vs[0] = p1;
    vs[1] = p2;
    vs[2] = p3;
    return vs;
  }
  std::array<Vertex, 3> to_array() const {
    std::array<Vertex, 3> vs;
    vs[0] = p1;
    vs[1] = p2;
    vs[2] = p3;
    return vs;
  }
  std::size_t size() const { return sizeof(Vertex) * 3; }
  VkDeviceSize dsize() const {
    return static_cast<VkDeviceSize>(size());
  }
  Vertex *data() const { return to_vector().data(); }
};
const std::vector<Vertex> vs = {
    {{0.0f, -0.5f}, {1.0f, 1.0f, 0.5f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.5f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

const Triangle triangle = Triangle(vs);
