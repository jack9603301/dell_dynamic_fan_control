#ifndef __FAN_SPEED_CALC_HPP
#define __FAN_SPEED_CALC_HPP

#include <cstdint>
#include <map>
#include <string>

class FanController {
public:
    typedef std::map<uint8_t, uint8_t> TemperatureMap_Type;
public:
    FanController(void) = default;
    FanController &operator=(const FanController &obj) = default; 
    
    void InsertTemperaturePoint(std::string curve_name, uint8_t temperature, uint8_t speed);
    uint8_t Inter(std::string curve_name, uint8_t target);
    uint8_t GetSpeedPWM(std::string curve_name, uint8_t temperature);
public:
    static uint8_t lineInter(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t x);
private:
    static void OutputLogsInfo(std::string str);
    static void OutputLogsWarning(std::string str);
    static void OutputLogsFatal(std::string str);

public:
    std::map<std::string, TemperatureMap_Type> CurveMaps;
};


#endif // #ifndef __FAN_SPEED_CALC_HPP
