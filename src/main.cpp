#include "Config.hpp"
#include "Global.hpp"
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <format>
#include "FanController.hpp"
#include "Devices/Devices.hpp"

namespace params_option = boost::program_options;

void show_version(void) {
    std::cout << std::format("{} Version {}.{}", TAG, MAJOR_VERSION, MINOR_VERSION) << std::endl;
}

int main(int argc, char **argv) {
    // Log system initialization
    boost::log::add_console_log(
        std::cout,
        boost::log::keywords::format =
            (boost::log::expressions::stream
            << boost::log::expressions::format_date_time<
                boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S")
            << " [" << boost::log::trivial::severity << "] "
            << boost::log::expressions::smessage));
    boost::log::add_common_attributes();

    params_option::options_description desc(
        "This is the smart home central control bus system.");
    desc.add_options()("help,h", "Display help information")(
        "config,c",
        params_option::value<std::string>()->default_value("config.yaml"),
        "Configuration file path (YAML format, default is config.yaml)")(
        "log-level,l", params_option::value<std::string>()->default_value("info"),
        "Log levels: trace, debug, info, warning, error, fatal")(
        "verbose,v", "Enable detailed output")(
        "version,V", "Show Version");

    params_option::variables_map vm;
    try {
        params_option::store(params_option::parse_command_line(argc, argv, desc),
                            vm);
        params_option::notify(vm);
    } catch (const params_option::error &e) {
        BOOST_LOG_TRIVIAL(fatal) << "[" << TAG << "] " << e.what();
        return 1;
    }

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    if (vm.count("version")) {
        show_version();
        return 0;
    }

    std::string config_path = vm["config"].as<std::string>();
    auto *config = Config::GetInstance();
    if (!config->LoadFromFile(config_path)) {
        BOOST_LOG_TRIVIAL(fatal)
            << "[" << TAG << "] "
            << "Unable to load configuration file: " << config_path;
        return 1;
    }
    BOOST_LOG_TRIVIAL(info) << "[" << TAG << "] "
                            << "Configuration file loaded: " << config_path;

    if (vm.count("log-level")) {
        std::string level_str = vm["log-level"].as<std::string>();
        boost::log::trivial::severity_level level;

        // String to severity_level mapping
        if (level_str == "trace") {
            level = boost::log::trivial::trace;
        } else if (level_str == "debug") {
            level = boost::log::trivial::debug;
        } else if (level_str == "info") {
            level = boost::log::trivial::info;
        } else if (level_str == "warning") {
            level = boost::log::trivial::warning;
        } else if (level_str == "error") {
            level = boost::log::trivial::error;
        } else if (level_str == "fatal") {
            level = boost::log::trivial::fatal;
        } else {
            BOOST_LOG_TRIVIAL(info)
                << "Unknown log level: " << level_str << ", Use the default info";
                level = boost::log::trivial::info;
        }

        // Set a global filter: only log entries with severity levels greater than
        // or equal to the specified level.
        boost::log::core::get()->set_filter(boost::log::trivial::severity >= level);
    }

    if (vm.count("verbose")) {
        config->SetBool("verbose", true);
    }

    BOOST_LOG_TRIVIAL(info) << "[" << TAG << "] " << "Log level: "
                          << config->GetString("log.level", "info");
    BOOST_LOG_TRIVIAL(info) << "[" << TAG << "] " << "Detailed mode: "
                          << (config->GetBool("verbose", false) ? "Enabled"
                                                                : "Disabled");
    FanController fan_controller;
    types::alias::CurveMap tempcurve = config->LoadTemperatureCurve();

    // Set temperature profile
    for (auto [curve_name, temperature_points] : tempcurve) {
        for (auto [temperature, speed] : temperature_points) {
            fan_controller.InsertTemperaturePoint(curve_name, temperature, speed);
        }
    }

    // Automatically register devices for the factory.
    devices::tools::AutoRegister();

    // Temperature monitoring
    if (!fan_controller.MonitorTemperature()) {
        return 1;
    }

    return 0;
}
