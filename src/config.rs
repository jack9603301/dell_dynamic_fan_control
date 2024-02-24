use std::fs::File;
use std::io::Read;
use serde::Deserialize;

#[derive(Deserialize)]
pub struct TemperaturePoint {
    pub temperature: u8,
    pub fan_speed: u8
}

#[derive(Deserialize)]
pub struct Setting {
    pub fan_num: u8,
    pub interval: u64
}

#[derive(Deserialize)]
pub struct TemperatureData {
    pub temperature_points: Vec<TemperaturePoint>,
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
