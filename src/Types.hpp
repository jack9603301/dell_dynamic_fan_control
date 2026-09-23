#ifndef __TYPES_HPP
#define __TYPES_HPP

#include <string>
#include <cstdint>
#include <map>
#include <memory>
#include <variant>
#include <vector>

class DeviceControlInterface;

namespace types {
namespace bases {
namespace fan {
namespace value_map {

struct AdvancedFanMapInfo {
    std::string speed_map;
    uint8_t refer;
    struct OffRefer{
        enum class ValueType{
            OFF,
            ON
        }type;
        uint8_t refer;
    }turn_off_refer;
};

struct AdvancedFanMapMetaInfo {
    std::string cpu_chip;
    std::vector<AdvancedFanMapInfo> speed_maps;
};

struct StaticFanMapInfo {
    uint8_t speed_map;
};

struct DynamicFanMapInfo {
    std::string cpu_chip;
    std::string speed_map;
};

}  // namespace value_map

struct FanMapInfo {
    uint8_t id;
    enum class MapType{
        STATIC,
        DYNAMIC,
        ADVANCED,
    }type;
    std::variant<value_map::StaticFanMapInfo, value_map::DynamicFanMapInfo, value_map::AdvancedFanMapMetaInfo> value;
};

}  // namespace fan
}  // namespace bases

namespace alias {

typedef std::map<uint8_t, uint8_t> TemperatureMap;
typedef std::map<std::string, TemperatureMap> CurveMap;
typedef std::shared_ptr<DeviceControlInterface> DeviceInterface;
typedef std::map<uint8_t, types::bases::fan::FanMapInfo> FanMap;

} // namespace alias
} // namespace types

#endif // #ifndef __TYPES_HPP
