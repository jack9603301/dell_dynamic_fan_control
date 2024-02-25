use std::fs::File;
use std::io::Read;
use serde::Deserialize;

#[derive(Deserialize)]
pub struct TemperaturePoint {
    pub temperature: u8,
    pub fan_speed: u8
}

#[derive(Deserialize)]
pub struct TemperaturePointMap {
    pub name: String,
    pub map: Vec<TemperaturePoint>
}

#[derive(Deserialize)]
pub struct FanMap {
    pub id: u8,
    pub static_fan_map: Option<u8>,
    pub dynamic_cpu_chip: Option<String>,
    pub dynamic_fan_speed_map: Option<String>
}

#[derive(Deserialize)]
pub struct Setting {
    pub fan_num: u8,
    pub interval: u64,
    pub fan_map: Option<Vec<FanMap>>
}

#[derive(Deserialize)]
pub struct TemperatureData {
    pub temperature_points: Option<Vec<TemperaturePointMap>>,
    pub setting: Setting
}

pub fn openYAML(filename: &str) -> String {
    let mut file = File::open(filename).expect("Failed to open file");
    let mut file_contents = String::new();
    file.read_to_string(&mut file_contents).expect("Failed to read file");
    return file_contents;
}

pub fn parse(contents: &mut String) -> TemperatureData {
    let data: TemperatureData = serde_yaml::from_str(&contents).expect("Failure to parse configuration file");
    return data;
}
