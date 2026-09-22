#include "DeviceControlInterface.hpp"
#include "Devices/Devices.hpp"
#include "Devices/Dell/DellControlUnit.hpp"

namespace devices {
namespace tools {

void AutoRegister(void) {
    DeviceControl *devices_control = DeviceControl::GetInstance();
    
    // Allocate resources to the equipment factory.
    dell::DellControlUnit *dell_control = new dell::DellControlUnit;

    // Auto Registers
    devices_control->Register(dell_control);
}

} // namespace bootstrap
} // namespace devices
