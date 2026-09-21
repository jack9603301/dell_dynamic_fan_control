#ifndef _HOME_HUB_CENTER_CONFIG
#define _HOME_HUB_CENTER_CONFIG

#include <string>
#include <yaml-cpp/yaml.h>
#include <unordered_map>
#include "FanController.hpp"
#include <map>

class Config {
public:
    typedef std::map<std::string, FanController::TemperatureMap_Type> CurveMap_Type;
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
    CurveMap_Type LoadTemperatureCurve(void);

private:
    Config(void);
    ~Config();

    void FlattenNode(const YAML::Node& node, const std::string& prefix, std::unordered_map<std::string, std::string>& flat_map);
    YAML::Node GetNode(const std::string& key) const;

    static void OutputLogsInfo(std::string str);
    static void OutputLogsWarning(std::string str);
    static void OutputLogsFatal(std::string str);

    YAML::Node root_node;
    std::unordered_map<std::string, std::string> flat_config;
    std::map<std::string, FanController::TemperatureMap_Type> CurveMaps;
};

#endif   // #ifndef _HOME_HUB_CENTER_CONFIG
