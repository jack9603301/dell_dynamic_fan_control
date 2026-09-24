#include "Devices/Dell/DellControlUnit.hpp"
#include <format>

namespace devices {
namespace dell {

std::string DellControlUnit::DeviceName(void) {
    return "dell";
}

bool DellControlUnit::operator()(uint8_t fanid, uint8_t speed) {
    std::string fanid_hex = std::format("{:02x}", fanid);
    std::string speed_hex = std::format("{:02x}", speed);

    // Constructing ipmitool commands
    // Disable BMC automatic fan control.
    std::string turn_off_auto_cmd = std::format(
    "ipmitool raw 0x30 0x30 0x01 0x00 > /dev/null 2>&1"
    );
    // Set target fan PWM
    std::string set_fan_cmd = std::format(
    "ipmitool raw 0x30 0x30 0x02 {} {} > /dev/null 2>&1", fanid_hex, speed_hex
    ); 

    // Execute the command and return the result.
    OutputLogsInfo(std::format("[devices:dell] Disabling IPMI automatic fan control command for Fan {}...", fanid));
    if (std::system(turn_off_auto_cmd.c_str()) != 0) {
        OutputLogsWarning(std::format("[devices:dell] Failed to send the command to disable IPMI automatic fan control for fan {}, command execution failed!", fanid));
        return false;
    }

    // Forced speed override
    OutputLogsInfo(std::format("[devices:dell] Forcing speed override on Fan {}, PWM = {}%...", fanid, speed));
    if (std::system(set_fan_cmd.c_str()) != 0) {
        OutputLogsWarning(std::format("[devices:dell] Forced speed override on Fan {} failed; command execution failed!", fanid));
        return false;
    }
    return true;
}

}  // namespace dell
} // namespace devices
