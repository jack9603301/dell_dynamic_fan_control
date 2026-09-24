#include "Global.hpp"
#include "Types.hpp"
#include <boost/log/trivial.hpp>
#include "Config.hpp"

Config *Config::GetInstance(void) {
    static Config *singleton = new Config;
    return singleton;
}
void Config::OutputLogsInfo(std::string str) {
    BOOST_LOG_TRIVIAL(info) << "[" << TAG << "] " << str;
}

void Config::OutputLogsWarning(std::string str) {
    BOOST_LOG_TRIVIAL(warning) << "[" << TAG << "] " << str;
}

void Config::OutputLogsFatal(std::string str) {
    BOOST_LOG_TRIVIAL(fatal) << "[" << TAG << "] " << str;
}

void Config::OutputLogsDebug(std::string str) {
    BOOST_LOG_TRIVIAL(debug) << "[" << TAG << "] " << str;
}

bool Config::LoadFromFile(const std::string& filepath) {
    try {
        root_node = YAML::LoadFile(filepath);
        flat_config.clear();
        FlattenNode(root_node, "", flat_config);
        return true;
    } catch (const YAML::Exception& e) {
        OutputLogsFatal("YAML parsing failed: " + std::string(e.what()));
        return false;
    }
}

void Config::FlattenNode(const YAML::Node& node, const std::string& prefix, std::unordered_map<std::string, std::string>& flat_map) {
    if (node.IsScalar()) {
        // Leaf node: Stores string values
        flat_map[prefix] = node.as<std::string>();
    } else if (node.IsSequence()) {
        // Sequence: Serializes to a YAML string (optional simplification).
        flat_map[prefix] = YAML::Dump(node);
    } else if (node.IsMap()) {
        // Mapping: Recursively traverse each child node
        for (auto it = node.begin(); it != node.end(); ++it) {
            std::string key = it->first.as<std::string>();
            std::string full_key = prefix.empty() ? key : prefix + "." + key;
            FlattenNode(it->second, full_key, flat_map);
        }
    }
}

YAML::Node Config::GetNode(const std::string& key) const {
    std::vector<std::string> key_segments;
    std::string current_segment;
    for (char c : key) {
        if (c == '.') {
            if (!current_segment.empty()) {
                key_segments.push_back(current_segment);
                current_segment.clear();
            }
        } else {
            current_segment += c;
        }
    }
    if (!current_segment.empty()) {
        key_segments.push_back(current_segment);
    }

    YAML::Node current = YAML::Clone(root_node);
    for (const std::string& seg : key_segments) {
        if (!current.IsDefined()) {
            return YAML::Node();
        }

        if (current.IsMap() && current[seg].IsDefined()) {
            current = YAML::Clone(current[seg]);
            continue;
        }

        if (current.IsSequence()) {
            try {
                size_t idx = std::stoul(seg);
                if (idx < current.size()) {
                    current = YAML::Clone(current[idx]);
                    continue;
                }
            } catch (...) {}
        }
        return YAML::Node();
    }

    return YAML::Clone(current);
}

std::string Config::GetString(const std::string& key, const std::string& default_val) const {
    // Prefer flattened cache (O(1))
    auto it = flat_config.find(key);
    if (it != flat_config.end()) {
        return it->second;
    }
    
    // Secondly, it is parsed in real time from YAML nodes for non-scalar types or uncached paths.
    YAML::Node node = GetNode(key);
    if (node.IsDefined() && node.IsScalar()) {
        return node.as<std::string>();
    }
    return default_val;
}

int Config::GetInt(const std::string& key, int default_val) const {
    // The values ​​in the flat cache are all strings and need to be converted.
    auto it = flat_config.find(key);
    if (it != flat_config.end()) {
        try {
            return std::stoi(it->second);
        } catch (...) {
            // The conversion failed, and the user reverted to the YAML node.
        }
    }
    YAML::Node node = GetNode(key);
    if (node.IsDefined() && node.IsScalar()) {
        return node.as<int>();
    }
    return default_val;
}

double Config::GetDouble(const std::string& key, double default_val) const {
    auto it = flat_config.find(key);
    if (it != flat_config.end()) {
        try {
            return std::stod(it->second);
        } catch (...) {}
    }
    YAML::Node node = GetNode(key);
    if (node.IsDefined() && node.IsScalar()) {
        return node.as<double>();
    }
    return default_val;
}

bool Config::GetBool(const std::string& key, bool default_val) const {
    auto it = flat_config.find(key);
    if (it != flat_config.end()) {
        // Supports common string representations
        std::string val = it->second;
        if (val == "true" || val == "yes" || val == "1") {
            return true;
        }
        if (val == "false" || val == "no" || val == "0") {
            return false;
        }
        // Otherwise, try integer conversion.
        try {
            return static_cast<bool>(std::stoi(val));
        } catch (...) {}
    }
    YAML::Node node = GetNode(key);
    if (node.IsDefined() && node.IsScalar()) {
        return node.as<bool>();
    }
    return default_val;
}

void Config::SetString(const std::string& key, const std::string& value) {
    // Update the flat cache (for faster reads).
    flat_config[key] = value;

    // Update the YAML node tree (for subsequent operations such as GetNode).
    // Creating/retrieving YAML nodes level by level
    YAML::Node node = root_node;
    std::string remaining = key;
    while (true) {
        auto dot_pos = remaining.find('.');
        std::string segment = remaining.substr(0, dot_pos);

        // If the key does not exist in the current layer, create a Map node.
        if (!node[segment]) {
            node[segment] = YAML::Node(YAML::NodeType::Map);
        }
        // Move to the next level
        node = node[segment];

        if (dot_pos == std::string::npos) {
            // The last key: Assign a scalar value
            node = value;       // Use overloaded assignment to convert to a scalar.
            break;
        }
        remaining = remaining.substr(dot_pos + 1);
    }
}
void Config::SetBool(const std::string& key, bool value) {
    // Convert the bool to a string and store it in a flat cache.
    flat_config[key] = value ? "true" : "false";

    // Update YAML nodes (assign bool type values)
    YAML::Node node = root_node;
    std::string remaining = key;
    while (true) {
        auto dot_pos = remaining.find('.');
        std::string segment = remaining.substr(0, dot_pos);
        if (!node[segment]) {
            node[segment] = YAML::Node(YAML::NodeType::Map);
        }
        node = node[segment];
        if (dot_pos == std::string::npos) {
            node = value;   // YAML-C++ automatically handles boolean types.
            break;
        }
        remaining = remaining.substr(dot_pos + 1);
    }
}

types::alias::CurveMap Config::LoadTemperatureCurve(void) {
    types::alias::CurveMap curve_result;

    // Get the root node of the temperature curve from the configuration.
    const YAML::Node temp_points_node = GetNode("temperature_points");
    if (!temp_points_node.IsDefined() || !temp_points_node.IsSequence()) {
        OutputLogsFatal("temperature_points is not a valid sequence in config");
        return curve_result;
    }

    // Iterate through all temperature curve entries.
    for (std::size_t idx = 0; idx < temp_points_node.size(); idx++) {
        const YAML::Node curve_entry = YAML::Clone(temp_points_node[idx]);

        // Extract curve name
        const std::string curve_name = curve_entry["name"].as<std::string>("");
        if (curve_name.empty()) {
            OutputLogsWarning("Skipping Curve with empty name at Index " + std::to_string(idx));
            continue;
        }

        // Extract the temperature-speed mapping list for the current curve.
        const YAML::Node curve_map_node = YAML::Clone(curve_entry["map"]);
        if (!curve_map_node.IsSequence()) {
            OutputLogsWarning("Curve '" + curve_name + "' has invalid 'map' field (not a sequence)");
            continue;
        }

        // Analyze all temperature-speed pairs on the current curve.
        types::alias::TemperatureMap current_curve;
        for (std::size_t pair_idx = 0; pair_idx < curve_map_node.size(); ++pair_idx) {
            const YAML::Node pair_node = curve_map_node[pair_idx];
            const int temp = pair_node["temperature"].as<int>();
            const int speed = pair_node["speed"].as<int>();

            if (temp >=0 && temp <=255 && speed >=0 && speed <=255) {
                current_curve.emplace(static_cast<uint8_t>(temp), static_cast<uint8_t>(speed));
            } else {
                OutputLogsWarning(
                    std::format("Ignoring invalid pair in Curve '{}' (Index {}): Temperature={}, PWM={}",
                                curve_name, pair_idx, temp, speed)
                );
            }
        }

        // Store the valid curve in the return result.
        if (!current_curve.empty()) {
            curve_result[curve_name] = current_curve;
            OutputLogsDebug(std::format("Loaded temperature Curve '{}' with {} Pairs", curve_name, current_curve.size()));
        } else {
            OutputLogsWarning("Curve '" + curve_name + "' has no valid temperature-speed pairs");
        }
    }

    // Final Log Statistics
    if (curve_result.empty()) {
        OutputLogsWarning("No valid temperature curves loaded from config");
    } else {
        OutputLogsInfo(std::format("Successfully loaded {} Temperature Curves total", curve_result.size()));
    }

    return curve_result;
}

types::alias::FanMap Config::LoadFanMapInfo(void) {
    types::alias::FanMap fan_map_result;
    const YAML::Node fan_map_node = GetNode("setting.fan_map");

    // Validate the validity of the configuration node.
    if (!fan_map_node.IsSequence()) {
        OutputLogsWarning("Invalid config: setting.fan_map is not a valid sequence");
        return fan_map_result;
    }

    // Iterate through each fan mapping configuration item.
    for (const YAML::Node& fan_item : fan_map_node) {
        types::bases::fan::FanMapInfo current_fan;

        if (!fan_item["id"].IsDefined()) {
            OutputLogsWarning("Skipping fan map item: missing required 'id' field");
            continue;
        }

        const uint8_t fan_id = static_cast<uint8_t>(fan_item["id"].as<int>());
        current_fan.id = fan_id;

        if (fan_item["static_speed_map"].IsDefined()) {
            current_fan.type = types::bases::fan::FanMapInfo::MapType::STATIC;
            uint8_t static_speed = static_cast<uint8_t>(fan_item["static_speed_map"].as<int>(0));
            current_fan.value = types::bases::fan::value_map::StaticFanMapInfo{static_speed};
            OutputLogsDebug(std::format("Parsed static fan rule: Id = {}, PWM = {}%", fan_id, static_speed));
        }
        else if (fan_item["dynamic_cpu_chip"].IsDefined() && fan_item["dynamic_speed_map"].IsDefined()) {
            // Advanced Mapping
            current_fan.type = types::bases::fan::FanMapInfo::MapType::DYNAMIC;
            std::string cpu_chip = fan_item["dynamic_cpu_chip"].as<std::string>("");
            std::string speed = fan_item["dynamic_speed_map"].as<std::string>("");
            current_fan.value = types::bases::fan::value_map::DynamicFanMapInfo{cpu_chip, speed};
            OutputLogsDebug(std::format("Parsed dynamic fan rule: Id = {}, Chip = {}, PWM = {}%", 
                fan_id, cpu_chip, speed));
        }
        else if (fan_item["dynamic_cpu_chip"].IsDefined() && fan_item["advanced_speed_map"].IsDefined()) {
            current_fan.type = types::bases::fan::FanMapInfo::MapType::ADVANCED;
            std::map<std::string, types::bases::fan::value_map::AdvancedFanMapInfo> adv_map;
            const YAML::Node& adv_node = YAML::Clone(fan_item["advanced_speed_map"]);

            if (!adv_node.IsSequence()) {
                OutputLogsWarning(std::format("Skipping advanced fan item Id = {}: advanced_speed_map is not a sequence", fan_id));
                continue;
            }
            
            std::vector<types::bases::fan::value_map::AdvancedFanMapInfo> adv_list;
            types::bases::fan::value_map::AdvancedFanMapMetaInfo adv_meta;
            for (std::size_t adv_idx = 0; adv_idx < adv_node.size(); ++adv_idx) {
                const YAML::Node& adv_entry = YAML::Clone(adv_node[adv_idx]);
                types::bases::fan::value_map::AdvancedFanMapInfo adv_info;

                uint8_t refer = static_cast<uint8_t>(adv_entry["refer"].as<int>(0));
                adv_info.speed_map = adv_entry["speed_map"].as<std::string>("default");
                adv_info.refer = refer;

                // Read the turn_off_refer switch (optional)
                if (adv_entry["turn_off_refer"].IsDefined()) {
                    uint8_t turn_off_refer = static_cast<uint8_t>(adv_entry["turn_off_refer"].as<int>(0));
                    if (turn_off_refer <= refer) {
                        adv_info.turn_off_refer.type = types::bases::fan::value_map::AdvancedFanMapInfo::OffRefer::ValueType::ON;
                        adv_info.turn_off_refer.refer = static_cast<uint8_t>(adv_entry["turn_off_refer"].as<int>(0));
                    } else {
                        OutputLogsWarning(std::format("In the advanced fan speed mapping rule (Rule {}) for Fan {}, the turn_off_refer threshold is higher than refer, triggering an automatic shutdown, Turn Off Refer = {}°C, Refer = {}°C.", adv_idx, fan_id, turn_off_refer, refer));
                        adv_info.turn_off_refer.type = types::bases::fan::value_map::AdvancedFanMapInfo::OffRefer::ValueType::OFF;
                        adv_info.turn_off_refer.refer = 0;
                    }
                } else {
                    adv_info.turn_off_refer.type = types::bases::fan::value_map::AdvancedFanMapInfo::OffRefer::ValueType::OFF;
                    adv_info.turn_off_refer.refer = 0;
                }

                adv_list.push_back(adv_info);
            }

            std::string cpu_chip = fan_item["dynamic_cpu_chip"].as<std::string>("");
            adv_meta.cpu_chip = cpu_chip;
            adv_meta.speed_maps = adv_list;
            current_fan.value = adv_meta;
            OutputLogsDebug(std::format("Parsed advanced fan rule: Id = {}, Entries = {}", fan_id, adv_list.size()));
        }
        else {
            OutputLogsWarning(std::format("Skipping fan rule Id = {}: no valid type (static/dynamic/advanced)", fan_id));
            continue;
        }

        // Add result mapping
        fan_map_result.emplace(fan_id, current_fan);
    }

    // Final Statistics Log
    if (fan_map_result.empty()) {
        OutputLogsWarning("No valid fan rules loaded from config");
    } else {
        OutputLogsInfo(std::format("Total parsed fan rules: {}", fan_map_result.size()));
    }

    return fan_map_result;
};
