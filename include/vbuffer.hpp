// vulkan buffer object
#pragma once

using namespace vtuto;

namespace vtuto {
template <class BufferType> class vulkan_buffer {
public:
  BufferType buffer;

public:
  vulkan_buffer() {}
};
}
