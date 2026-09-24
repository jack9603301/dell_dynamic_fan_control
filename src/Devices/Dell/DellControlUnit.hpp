#ifndef __DEVICES_DELL_DELL_CONTROL_UNIT_HPP
#define __DEVICES_DELL_DELL_CONTROL_UNIT_HPP

#include "DeviceControlInterface.hpp"
#include <cstdint>

namespace devices {
namespace dell {

class DellControlUnit : public DeviceControlInterface {
public:
    virtual std::string DeviceName(void);
    virtual void Initialization(void);
    virtual bool Destroy(void);
    virtual bool operator()(uint8_t fanid, uint8_t speed);
};

} // namespace dell
}  // namespace devices

#endif // #ifndef __DEVICES_DELL_DELL_CONTROL_UNIT_HPP
