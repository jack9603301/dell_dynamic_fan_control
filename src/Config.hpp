#ifndef _CONFIG_HPP
#define _CONFIG_HPP

#include "Global.hpp"
#include "Types.hpp"
#include <string>
#include <yaml-cpp/yaml.h>
#include <unordered_map>
#include <map>

class Config { 
public:
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    static Config *GetInstance(void);
    bool LoadFromFile(const std::string& filepath);
    void SetString(const std::string& key, const std::string& value);
    void SetBool(const std::string& key, bool value);
    std::string GetString(const std::string& key, const std::string& default_val = "") const;
    int GetInt(const std::string& key, int default_val = 0) const;
    double GetDouble(const std::string& key, double default_val = 0.0) const;
    bool GetBool(const std::string& key, bool default_val = false) const;
    types::alias::CurveMap LoadTemperatureCurve(void);
    std::map<uint8_t, types::bases::fan::FanMapInfo> LoadFanMapInfo(void);

private:
    Config(void) = default;
    ~Config() = default;

    void FlattenNode(const YAML::Node& node, const std::string& prefix, std::unordered_map<std::string, std::string>& flat_map);
    YAML::Node GetNode(const std::string& key) const;

    static void OutputLogsInfo(std::string str);
    static void OutputLogsWarning(std::string str);
    static void OutputLogsFatal(std::string str);
    static void OutputLogsDebug(std::string str);

    YAML::Node root_node;
    std::unordered_map<std::string, std::string> flat_config;
    std::map<std::string, types::alias::TemperatureMap> CurveMaps;
};

#endif   // #ifndef _CONFIG_HPP
