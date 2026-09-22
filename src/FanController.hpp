#ifndef __FAN_CONTROLLER_HPP
#define __FAN_CONTROLLER_HPP

#include "Global.hpp"
#include <cstdint>
#include <map>
#include <string>
#include "Config.hpp"

class FanController { 
public:
    FanController(void);
    FanController &operator=(const FanController &obj) = default; 
    
    void InsertTemperaturePoint(std::string curve_name, uint8_t temperature, uint8_t speed);
    uint8_t Inter(std::string curve_name, uint8_t target);
    uint8_t GetSpeedPWM(std::string curve_name, uint8_t temperature);
    bool MonitorTemperature(void);
public:
    static uint8_t lineInter(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t x);
private:
    static void OutputLogsInfo(std::string str);
    static void OutputLogsWarning(std::string str);
    static void OutputLogsFatal(std::string str);

public:
    std::map<std::string, types::alias::TemperatureMap> CurveMaps;
    Config *config;
};


#endif // #ifndef __FAN_CONTROLLER_HPP
