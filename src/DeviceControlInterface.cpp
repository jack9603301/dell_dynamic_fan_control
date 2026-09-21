#include "Global.hpp"
#include "DeviceControlInterface.hpp"
#include <boost/log/trivial.hpp>

DeviceControl *DeviceControl::instance = nullptr;

void DeviceControl::OutputLogsInfo(std::string str) {
    BOOST_LOG_TRIVIAL(info) << "[" << TAG << "] " << str;
}

void DeviceControl::OutputLogsWarning(std::string str) {
    BOOST_LOG_TRIVIAL(fatal) << "[" << TAG << "] " << str;
}


void DeviceControl::OutputLogsFatal(std::string str) {
    BOOST_LOG_TRIVIAL(fatal) << "[" << TAG << "] " << str;
}

DeviceControl *DeviceControl::GetInstance(void) {
    if (this->instance == nullptr) {
        this->instance = new DeviceControl;
    }

    return this->instance;
}

bool DeviceControl::Register(DeviceInterface_Type interface) {
    if (this->RegisterMaps.empty() || this->RegisterMaps.find(interface->DeviceName()) == this->RegisterMaps.end()) {
        this->RegisterMaps.emplace(interface->DeviceName(), interface);
        return true;
    }

    return false;
}

bool DeviceControl::UnRegister(std::string device_name) {
    if (!this->RegisterMaps.empty() && this->RegisterMaps.find(device_name) != this->RegisterMaps.end()) {
        this->RegisterMaps.erase(device_name);
    }
    return false;
}

bool DeviceControl::UnRegister(DeviceInterface_Type interface) {
    if (!this->RegisterMaps.empty() && this->RegisterMaps.find(interface->DeviceName()) != this->RegisterMaps.end()) {
        this->RegisterMaps.erase(interface->DeviceName());
    }
    return false;
}
