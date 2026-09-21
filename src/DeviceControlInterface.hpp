#ifndef __DEVICE_CONTROL_INTERFACE_HPP
#define __DEVICE_CONTROL_INTERFACE_HPP

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
    virtual bool operator()(uint8_t speed) = 0;
};

class DeviceControl {
public:
    typedef std::shared_ptr<DeviceControlInterface> DeviceInterface_Type;
private:
    DeviceControl(void) = default;
    DeviceControl(const DeviceControl &obj) = default;
    DeviceControl &operator=(const DeviceControl &obj) = default;
    ~DeviceControl() = default;
public:
    static void OutputLogsInfo(std::string str);
    static void OutputLogsWarning(std::string str);
    static void OutputLogsFatal(std::string str);

    DeviceControl *GetInstance(void);
    bool Register(DeviceControlInterface *interface);
    bool UnRegister(std::string device_name);
    bool operator()(std::string device_name, uint8_t speed);
private:
    static DeviceControl *instance;
    std::map<std::string, DeviceInterface_Type> RegisterMaps;
};

#endif // #ifndef __DEVICE_CONTROL_INTERFACE_HPP
