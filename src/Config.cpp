#include "Global.hpp"
#include <boost/log/trivial.hpp>
#include "Config.hpp"

Config::Config(void) {
    OutputLogsInfo("Config singleton initialized.");
}
Config::~Config() {
    OutputLogsInfo("Config singleton uninitialized.");
}

Config *Config::GetInstance(void) {
    static Config *singleton = new Config;
    OutputLogsInfo("Get the instance address of the configuration module");
    return singleton;
}
void Config::OutputLogsInfo(std::string str) {
    BOOST_LOG_TRIVIAL(info) << "[" << TAG << "] " << str;
}

void Config::OutputLogsWarning(std::string str) {
    BOOST_LOG_TRIVIAL(fatal) << "[" << TAG << "] " << str;
}


void Config::OutputLogsFatal(std::string str) {
    BOOST_LOG_TRIVIAL(fatal) << "[" << TAG << "] " << str;
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
    // Starting from the root node, search level by level along the path separated by dots.
    YAML::Node node = root_node;
    std::string remaining = key;

    while (!remaining.empty()) {
        // Find the next breakpoint in the current level
        auto dot_pos = remaining.find('.');
        std::string segment = remaining.substr(0, dot_pos);

        // If the current node is not a Map or has no child key, return an empty node.
        if (!node.IsMap() || !node[segment]) {
            return YAML::Node();
        }

        // Descend to child node
        node = node[segment];

        // If the end of the path has been reached, return to the current node.
        if (dot_pos == std::string::npos) {
            break;
        }

        // Continue processing the remaining paths
        remaining = remaining.substr(dot_pos + 1);
    }

    return node;
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

Config::CurveMap_Type Config::LoadTemperatureCurve(void) {
    CurveMap_Type curve_result;

    // Get the root node of the temperature curve from the configuration.
    const YAML::Node temp_points_node = GetNode("temperature_points");
    if (!temp_points_node.IsSequence()) {
        OutputLogsFatal("temperature_points is not a valid sequence in config");
        return curve_result;
    }

    // Iterate through all temperature curve entries.
    for (std::size_t idx = 0; idx < temp_points_node.size(); idx++) {
        const YAML::Node curve_entry = temp_points_node[idx];

        // Extract curve name
        const std::string curve_name = curve_entry["name"].as<std::string>("");
        if (curve_name.empty()) {
            OutputLogsWarning("Skipping curve with empty name at index " + std::to_string(idx));
            continue;
        }

        // Extract the temperature-speed mapping list for the current curve.
        const YAML::Node curve_map_node = curve_entry["map"];
        if (!curve_map_node.IsSequence()) {
            OutputLogsWarning("Curve '" + curve_name + "' has invalid 'map' field (not a sequence)");
            continue;
        }

        // Analyze all temperature-speed pairs on the current curve.
        FanController::TemperatureMap_Type current_curve;
        for (std::size_t pair_idx = 0; pair_idx < curve_map_node.size(); ++pair_idx) {
            const YAML::Node pair_node = curve_map_node[pair_idx];
            const int temp = pair_node["temperature"].as<int>(-1);
            const int speed = pair_node["fan_speed"].as<int>(-1);

            if (temp >=0 && temp <=255 && speed >=0 && speed <=255) {
                current_curve.emplace(static_cast<uint8_t>(temp), static_cast<uint8_t>(speed));
            } else {
                OutputLogsWarning(
                    std::format("Ignoring invalid pair in curve '{}' (index {}): temp={}, speed={}",
                                curve_name, pair_idx, temp, speed)
                );
            }
        }

        // Store the valid curve in the return result.
        if (!current_curve.empty()) {
            curve_result[curve_name] = std::move(current_curve);
            OutputLogsInfo(std::format("Loaded temperature curve '{}' with {} pairs", curve_name, current_curve.size()));
        } else {
            OutputLogsWarning("Curve '" + curve_name + "' has no valid temperature-speed pairs");
        }
    }

    // Final Log Statistics
    if (curve_result.empty()) {
        OutputLogsWarning("No valid temperature curves loaded from config");
    } else {
        OutputLogsInfo(std::format("Successfully loaded {} temperature curves total", curve_result.size()));
    }

    return curve_result;
}
