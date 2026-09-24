#ifndef __DEVICE_CONTROL_INTERFACE_HPP
#define __DEVICE_CONTROL_INTERFACE_HPP

#include "Global.hpp"
#include <string>
#include <memory>
#include <map>
#include <cstdint>

class DeviceControlInterface {
public:
    DeviceControlInterface(void) = default;
    DeviceControlInterface(const DeviceControlInterface &obj) = delete;
    DeviceControlInterface &operator=(const DeviceControlInterface &obj) = delete;
public:
    virtual std::string DeviceName(void) = 0;
    virtual bool operator()(uint8_t fanid, uint8_t speed) = 0;
public:
    static void OutputLogsInfo(std::string str);
    static void OutputLogsWarning(std::string str);
    static void OutputLogsFatal(std::string str);
    static void OutputLogsDebug(std::string str);
};

class DeviceControl { 
private:
    DeviceControl(void) = default;
    DeviceControl(const DeviceControl &obj) = default;
    DeviceControl &operator=(const DeviceControl &obj) = default;
    ~DeviceControl() = default;
public:
    static void OutputLogsInfo(std::string str);
    static void OutputLogsWarning(std::string str);
    static void OutputLogsFatal(std::string str);
    static void OutputLogsDebug(std::string str);

    static DeviceControl *GetInstance(void);
    bool Register(DeviceControlInterface *interface);
    bool UnRegister(std::string device_name);
    bool operator()(std::string device_name, uint8_t fanid, uint8_t speed);
private:
    static DeviceControl *instance;
    std::map<std::string, types::alias::DeviceInterface> RegisterMaps;
};

#endif // #ifndef __DEVICE_CONTROL_INTERFACE_HPP
