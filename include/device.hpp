// device template for physical and logical devices
#pragma once

namespace vtuto {
template <class VDeviceType, class DeviceType>
class vulkan_device {
public:
  VDeviceType device(const DeviceType &dev);
  void destroy(const DeviceType &dev) {}
};
}
