#include "DeviceControlInterface.hpp"
#include "Global.hpp"
#include "FanController.hpp"
#include <boost/log/trivial.hpp>
#include <chrono>
#include <format>
#include "Config.hpp"
#include "Types.hpp"
#include <boost/assert.hpp>
#include <sensors/sensors.h>
#include <thread>
#include <stop_token>

extern std::stop_source stop_source;

void FanController::OutputLogsInfo(std::string str) {
    BOOST_LOG_TRIVIAL(info) << str;
}

void FanController::OutputLogsWarning(std::string str) {
    BOOST_LOG_TRIVIAL(warning) << str;
}

void FanController::OutputLogsFatal(std::string str) {
    BOOST_LOG_TRIVIAL(fatal) << str;
}

void FanController::OutputLogsDebug(std::string str) {
    BOOST_LOG_TRIVIAL(debug) << str;
}

void FanController::InsertTemperaturePoint(std::string curve_name, uint8_t temperature, uint8_t speed) {
    types::alias::TemperatureMap temperature_points;
    // Check for the existence of a temperature profile mapping.
    if (!this->CurveMaps.empty() && this->CurveMaps.find(curve_name) != this->CurveMaps.end()) {
        temperature_points = this->CurveMaps[curve_name];
    }
    
    temperature_points.emplace(temperature, speed);
    this->CurveMaps[curve_name] = temperature_points;
    OutputLogsDebug(std::format("Generate speed mapping curve information, Curve = {}, Temperature = {}°C, PWM = {}%!", curve_name, temperature, speed));
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
    if (CurveMaps.empty() || CurveMaps.find(curve_name) == CurveMaps.end()) {
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

    uint8_t prev_temp = 0;
    uint8_t prev_speed = 0;

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

bool FanController::MonitorTemperature(void) {
    // Initialize the lm-sensors library.
    if (sensors_init(nullptr) != 0) {
        OutputLogsFatal("Failed to initialize lm-sensors library");
        return false;
    }

    //Dynamically retrieve fan-related parameters from the configuration.
    const int fan_num = config->GetInt("setting.fan_num");
    const int interval = config->GetInt("setting.interval", 60);

    if (fan_num == 0) {
        OutputLogsFatal("Total number of fans is 0.");
        sensors_cleanup();
        return false;
    }

    const auto fan_map_result = config->LoadFanMapInfo();
    const auto curve_map = config->LoadTemperatureCurve();

    std::jthread monitor_thread([this, &fan_num, &interval, &fan_map_result, &curve_map](std::stop_token stoken) {
        auto changed_fan_speed = [this](uint8_t fanid, uint8_t pwm) {
            DeviceControl *device_control = DeviceControl::GetInstance();

            std::string device = this->config->GetString("setting.device");
            if (device.empty()) {
                OutputLogsWarning(std::format("Due to a lack of target device type, it is impossible to send a forced override command to control the speed of Fan {}, PWM = {}%", fanid, pwm));
            } else {
                OutputLogsDebug(std::format("Send a command to the Fan Control Unit of Device {} to force the speed of Fan {} to {}%.", device, fanid, pwm));
                (*device_control)(device, fanid, pwm);
            }
        };

        OutputLogsDebug("Temperature monitor thread started");
        
        std::map<uint8_t, types::bases::fan::value_map::AdvancedFanMapInfo> advanced_speed_cache;

        // Device Initialization
        DeviceControl *device_control = DeviceControl::GetInstance();
        std::string device = this->config->GetString("setting.device");
        device_control->Initialization(device);

        while(!stoken.stop_requested()) {
            std::unordered_map<std::string, uint8_t> chip_temp_map;
            int chip_idx = 0;
            const ::sensors_chip_name* chip = nullptr;

            while ((chip = ::sensors_get_detected_chips(nullptr, &chip_idx)) != nullptr) {
                char chip_name_buf[256];
                std::string chip_name;
                if (::sensors_snprintf_chip_name(chip_name_buf, sizeof(chip_name_buf), chip) > 0) {
                    chip_name = chip_name_buf;
                } else {
                    continue;
                }

                const sensors_feature* feature = nullptr;
                uint8_t chip_max_temp = 0;
                int feature_idx = 0;

                while ((feature = ::sensors_get_features(chip, &feature_idx)) != nullptr) {
                    feature_idx++;
                    if (feature->type != SENSORS_FEATURE_TEMP) {
                        continue;
                    }

                    
                    sensors_subfeature_type sub_type = sensors_subfeature_type::SENSORS_SUBFEATURE_TEMP_INPUT;
                    const sensors_subfeature* sub = ::sensors_get_subfeature(chip, feature, sub_type);
                    double temp_val = 0;
                    if (sensors_get_value(chip, sub->number, &temp_val) == 0) {
                        uint8_t core_temp = static_cast<uint8_t>(std::round(temp_val));
                        if (core_temp > chip_max_temp) {
                            chip_max_temp = core_temp;
                        }
                    }
                }

                if (chip_max_temp > 0) {
                    chip_temp_map[chip_name] = chip_max_temp;
                    OutputLogsInfo(std::format("Detected CPU Chip: {}, Max Temperature: {}°C", chip_name, chip_max_temp));
                }
            }
            
            // Calculate PWM based on mapping rules.
            for (uint8_t fan_id = 0; fan_id < static_cast<uint8_t>(fan_num); ++fan_id) {
                uint8_t target_pwm = 0;

                // Match the mapping rules for the current fan.
                if (fan_map_result.count(fan_id)) {
                    const auto& fan_info = fan_map_result.at(fan_id);
                    switch (fan_info.type) {
                    case types::bases::fan::FanMapInfo::MapType::STATIC: {
                            const auto& static_val = std::get<types::bases::fan::value_map::StaticFanMapInfo>(fan_info.value);
                            target_pwm = static_val.speed_map;
                            OutputLogsDebug(std::format("Fan {}: Static mapping, PWM = {}%", fan_id, target_pwm));
                            break;
                        }
                    case types::bases::fan::FanMapInfo::MapType::DYNAMIC: {
                            const auto& dynamic_val = std::get<types::bases::fan::value_map::DynamicFanMapInfo>(fan_info.value);
                            const std::string& target_chip = dynamic_val.cpu_chip;
                            const std::string& speed_map_name = dynamic_val.speed_map;

                            // Match detected CPU chips
                            if (chip_temp_map.count(target_chip)) {
                                uint8_t chip_temp = chip_temp_map[target_chip];
                                target_pwm = this->GetSpeedPWM(speed_map_name, chip_temp);
                                OutputLogsDebug(std::format("Fan {}: Dynamic mapping, Chip = {}, Curve = {}, PWM = {}%", 
                                    fan_id, target_chip, speed_map_name, target_pwm));
                            } else {
                                OutputLogsWarning(std::format("Fan {}: Dynamic Chip {} not found, fallback to default Curve", fan_id, target_chip));
                            }
                            break;
                        }
                    case types::bases::fan::FanMapInfo::MapType::ADVANCED: {
                            const auto& adv_meta = std::get<types::bases::fan::value_map::AdvancedFanMapMetaInfo>(fan_info.value);
                            const auto& adv_list = adv_meta.speed_maps;
                            const std::string& target_chip = adv_meta.cpu_chip;

                            if (!target_chip.empty() && !chip_temp_map.count(target_chip)) {
                                OutputLogsWarning(std::format("Fan {}: Advanced chip {} not found, fallback to default", fan_id, target_chip));
                                break;
                            }
                            uint8_t current_temp = target_chip.empty() ? 0 : chip_temp_map[target_chip];
                            bool dynamic_match = true;

                            auto temperature_policy = [this, &changed_fan_speed, &current_temp, &advanced_speed_cache, &fan_id, &target_pwm, adv_list, adv_meta](bool allow_descent = true, types::bases::fan::value_map::AdvancedFanMapInfo *descent_lock = nullptr) {
                                // Handling the 'refer' threshold for advanced mappings: Identify the highest-priority mapping corresponding to the current temperature.
                                uint8_t max_refer = 0;
                                std::string selected_curve = "default";
                                bool need_update_advance = false;

                                // Advanced mapping logic: Iterate through the reference data and match the rule corresponding to the current temperature.
                                for (const auto& adv : adv_list) {
                                    if (current_temp >= adv.refer && current_temp >= max_refer) {
                                        if (!allow_descent && descent_lock != nullptr) {
                                            // Execute state machine descent strategy lock.
                                            if (current_temp <= descent_lock->refer) {
                                                need_update_advance = false;
                                                OutputLogsDebug(std::format("Transfer to a lower temperature is not possible because the current temperature is below the activation lock threshold. Current Temperature = {}°C, Locked Refer = {}, Locked Turn Off Refer = {}, Locked Curve = {}", 
                                                    current_temp,
                                                    descent_lock->refer, 
                                                    descent_lock->turn_off_refer.type == types::bases::fan::value_map::AdvancedFanMapInfo::OffRefer::ValueType::ON ?
                                                        std::format("{}",descent_lock->turn_off_refer.refer) : std::string("<OFF>"),
                                                    descent_lock->speed_map));
                                                continue;
                                            }
                                        }
                                        max_refer = adv.refer;
                                        selected_curve = adv.speed_map;
                                        advanced_speed_cache[fan_id] = adv;
                                        need_update_advance = true;
                                    }
                                }

                                if (need_update_advance && max_refer >= 0) {
                                    this->fanmap_advanced_rules.emplace(fan_id, advanced_speed_cache[fan_id]);
                                    target_pwm = this->GetSpeedPWM(selected_curve, current_temp);
                                    if (!allow_descent && descent_lock != nullptr) {
                                        OutputLogsInfo(std::format("Fan {}: Advanced mapping configuration locked, Refer = {}°C, Curve = {}, PWM = {}%",
                                            fan_id, max_refer, selected_curve, target_pwm));
                                    } else {
                                        OutputLogsInfo(std::format("Fan {}: Advanced mapping, Refer = {}°C, Curve = {}, PWM = {}%", 
                                            fan_id, max_refer, selected_curve, target_pwm));
                                    }
                                }
                            };


                            if (this->fanmap_advanced_rules.find(fan_id) != this->fanmap_advanced_rules.end()) {
                                // Existing configurations exist in the system.
                                types::bases::fan::value_map::AdvancedFanMapInfo system_adv_rule = this->fanmap_advanced_rules[fan_id];

                                if (system_adv_rule.turn_off_refer.type == types::bases::fan::value_map::AdvancedFanMapInfo::OffRefer::ValueType::ON && 
                                        current_temp <= system_adv_rule.turn_off_refer.refer) {
                                    this->fanmap_advanced_rules.erase(fan_id);   // Exit conditions met; clearing configuration lock.
                                    OutputLogsDebug(std::format("Fan {} meets the shutdown/exit conditions for Rule {}, the configuration lock for the advanced mapping rule is cleared.", fan_id, system_adv_rule.speed_map));
                                    dynamic_match = true;
                                } else if(system_adv_rule.turn_off_refer.type == types::bases::fan::value_map::AdvancedFanMapInfo::OffRefer::ValueType::OFF &&
                                        current_temp <= system_adv_rule.refer) {
                                    this->fanmap_advanced_rules.erase(fan_id);   // Exit conditions met; clearing configuration lock.
                                    OutputLogsDebug(std::format("Fan {} meets the shutdown/exit conditions for Rule {}, the configuration lock for the advanced mapping rule is cleared.", fan_id, system_adv_rule.speed_map));
                                } else {
                                    temperature_policy(false, &system_adv_rule);
                                    dynamic_match = false;
                                }
                            }

                            if (dynamic_match) {
                                temperature_policy(true, nullptr);
                            }
                            break;
                        }
                    default:
                        break;
                    }

                    changed_fan_speed(fan_id, target_pwm);
                    OutputLogsDebug(std::format("Fan {}: Set PWM to {}%", fan_id, target_pwm));
                } 
            }

            auto wait_end = std::chrono::steady_clock::now() + std::chrono::seconds(interval);
            while (std::chrono::steady_clock::now() < wait_end) {
                if (stoken.stop_requested()) { 
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(STOP_TOKEN_WAITFOR_MSTIMEOUT));
            }

            std::this_thread::sleep_for(std::chrono::seconds(interval));
        }
    }, stop_source.get_token());

    // Thread startup
    if (monitor_thread.joinable()) {
        monitor_thread.join();

        // Device Destory
        DeviceControl *device_control = DeviceControl::GetInstance();
        std::string device = this->config->GetString("setting.device");
        device_control->Destroy(device);
    } else {
        OutputLogsFatal("Failed to create temperature monitor thread");
        sensors_cleanup();
        return false;
    }

    sensors_cleanup();
    OutputLogsDebug("Temperature monitor task completed");

    return true;
}

FanController::FanController(void) : config(Config::GetInstance()) {
}
