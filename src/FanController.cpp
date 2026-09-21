#include "Global.hpp"
#include "FanController.hpp"
#include <boost/log/trivial.hpp>
#include <format>
#include "Config.hpp"
#include <boost/assert.hpp>

void FanController::OutputLogsInfo(std::string str) {
    BOOST_LOG_TRIVIAL(info) << "[" << TAG << "] " << str;
}

void FanController::OutputLogsWarning(std::string str) {
    BOOST_LOG_TRIVIAL(warning) << "[" << TAG << "] " << str;
}

void FanController::OutputLogsFatal(std::string str) {
    BOOST_LOG_TRIVIAL(fatal) << "[" << TAG << "] " << str;
}

void FanController::InsertTemperaturePoint(std::string curve_name, uint8_t temperature, uint8_t speed) {
    TemperatureMap_Type temperature_points;
    // Check for the existence of a temperature profile mapping.
    if (!this->CurveMaps.empty() && this->CurveMaps.find(curve_name) != this->CurveMaps.end()) {
        temperature_points = this->CurveMaps[curve_name];
    }
    
    temperature_points.emplace(temperature, speed);
    this->CurveMaps[curve_name] = temperature_points;
    OutputLogsInfo(std::format("Generate speed mapping curve information, curve_name: {}, Temperature: {}, Speed: {}!", curve_name, temperature, speed));
}

uint8_t FanController::lineInter(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t x) {
    if (x0 == x1) {
        return y0;
    }

    double slope = (static_cast<double>(y1) - y0) / (static_cast<double>(x1) - x0);
    double intercept = static_cast<double>(y0) - slope * static_cast<double>(x0);
    double target = slope * static_cast<double>(x) + intercept;
    return static_cast<uint8_t>(target);
}

uint8_t FanController::Inter(std::string curve_name, uint8_t target) {
    if (CurveMaps.empty() || CurveMaps.find(curve_name) != CurveMaps.end()) {
        OutputLogsFatal("Temperature speed mapping is empty");
        exit(1);
        return 0;
    }

    if (CurveMaps[curve_name].empty()) {
        OutputLogsFatal("Temperature speed mapping is empty");
        exit(1);
        return 0;
    }

    BOOST_ASSERT_MSG(!CurveMaps[curve_name].empty(), "Temperature profile configuration data does not exist.");

    auto it = CurveMaps[curve_name].begin();
    uint8_t prev_temp = it->first;
    uint8_t prev_speed = it->second;

    if (target <= prev_temp) {
        return prev_speed;
    }

    for (auto [curr_temp, curr_speed] : this->CurveMaps[curve_name]) {
        if (target <= curr_temp) {
            return lineInter(prev_temp, prev_speed, curr_temp, curr_speed, target);
        }

        prev_temp = curr_temp;
        prev_speed = curr_speed;
    }
    return prev_speed;
}

uint8_t FanController::GetSpeedPWM(std::string curve_name, uint8_t temperature) {
    return Inter(curve_name, temperature);
}

