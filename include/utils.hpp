#pragma once

#include <external.hpp>

#define CHECK_VK(call, msg)                                \
  {                                                        \
    VkResult res = call;                                   \
    if (res != VK_SUCCESS) {                               \
      std::stringstream ss;                                \
      ss << "Error in: " << __FILE__ << " :: " << __LINE__ \
         << " :: " << #call << " :: " << msg << std::endl; \
      std::string s = ss.str();                            \
      throw std::runtime_error(s);                         \
    }                                                      \
  }
