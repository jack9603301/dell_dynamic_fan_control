extern crate lm_sensors;
extern crate log;

mod config;
mod calc_temp_fan;
use syslog::{Facility, Formatter3164, BasicLogger};
use log::{SetLoggerError, LevelFilter, info};

use std::env;
use std::thread::sleep;
use std::time::Duration;
use std::process::Command;

fn find_temp_fan_speed_map<'a>(data: &'a config::TemperatureData, name: & String) -> Result<&'a Vec<config::TemperaturePoint>, String> {
    if data.temperature_points.is_none() {
        return Err("No temperature point defined!".to_string());
    }
    let temp_map_vec = data.temperature_points.as_ref().unwrap();
    for map in temp_map_vec {
        if map.name == *name {
            return Ok(map.map.as_ref());
        }
    }
    Err("No temperature point defined!".to_string())
}

fn changed_fan_speed(fanid: u8, pwm: u8) {
    let fanid_str = format!("0x{:02x}", fanid);
    let pwm_str = format!("0x{:02x}", pwm);
    Command::new("ipmitool").arg("raw").arg("0x30").arg("0x30").arg("0x01").arg("0x00").output().expect("Failed to turn off the automatic control of the BMC fan!");
    Command::new("ipmitool").arg("raw").arg("0x30").arg("0x30").arg("0x02").arg(fanid_str).arg(pwm_str).output().expect("Failed to set fan control parameters!");
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let filename = match args.get(1) {
        Some(arg) => arg,
        None => {
            eprintln!("Provide configuration program file names");
            std::process::exit(1);
        }
    };
    let mut contents = config::openYAML(filename);
    let data = config::parse(&mut contents);
    calc_temp_fan::clear_data();
    //calc_temp_fan::forset_data(&data);

    let sensors = lm_sensors::Initializer::default().initialize().expect("Sensor initialization failure!");

    let formatter = Formatter3164 {
        facility: Facility::LOG_USER,
        hostname: Some(String::from("R720")),
        process: "dynamic_fan_control".into(),
        pid: 0,
    };

    let logger = match syslog::unix(formatter) {
        Err(e) => { println!("impossible to connect to syslog: {:?}", e); return; },
        Ok(logger) => logger,
    };
    let _ = log::set_boxed_logger(Box::new(BasicLogger::new(logger)))
            .map(|()| log::set_max_level(LevelFilter::Info));

    let fan_num = data.setting.fan_num;
    let interval = data.setting.interval;

    println!("Detected {} fans!", fan_num);
    info!("Detected {} fans!", fan_num);

    println!("Perform detection every {} seconds!", interval);
    info!("Perform detection every {} seconds!", interval);

    loop {
        for chip in sensors.chip_iter(None) {
            if chip.path().is_some() {
                let name: String = chip.name().expect("Failed to get chip name!");
                if name.contains("coretemp") {
                    let mut coretemp: f64 = 0.0;
                    println!("CPU Chip: Checking, {}!",chip);
                    for feature in chip.feature_iter() {
                        for sub_feature in feature.sub_feature_iter() {
                            if let Ok(value) = sub_feature.value() {
                                let subname: String = match sub_feature.name() {
                                    Some(name) => name.expect("Failed to get subfeature name!").to_string(),
                                    None => {
                                        eprintln!("Can't get subfeature name!");
                                        std::process::exit(1);
                                    }
                                };
                                if subname.contains("input") {
                                    println!("Check Temperature as {}: {}!", chip, value);
                                    if coretemp < value.raw_value() {
                                        coretemp = value.raw_value();
                                    }
                                }
                            }
                        }
                    }
                    let temp = coretemp as u8;
                    let mut info_log = format!("Maximum temperature detected of {}: {}!", name, temp);
                    println!("{}", info_log);
                    info!("{}", info_log);
                    for i in 0..fan_num {
                        let fan_map = &data.setting.fan_map;
                        let mut found = false;
                        match fan_map {
                            Some(fan_map) => {
                                let n = fan_map.len();
                                for j in 0..n {
                                    let id = fan_map[j].id;
                                    if id != i {
                                        continue;
                                    }
                                    found = true;
                                    if fan_map[j].static_fan_map.is_some() {
                                        continue;
                                    }
                                    if fan_map[j].dynamic_cpu_chip.is_none() || (fan_map[j].dynamic_fan_speed_map.is_none() && fan_map[j].advanced_speed_map.is_none()) {
                                        continue
                                    }
                                    let chip_name = fan_map[j].dynamic_cpu_chip.as_ref().unwrap();
                                    if *chip_name != name {
                                        continue;
                                    }
                                    let mut fan_speed_map: String = String::new();
                                    if fan_map[j].dynamic_fan_speed_map.is_some() {
                                        fan_speed_map = fan_map[j].dynamic_fan_speed_map.as_ref().unwrap().clone();
                                    } else {
                                        info_log = format!("Detecting the advanced speed of fan {}", i);
                                        println!("{}", info_log);
                                        info!("{}", info_log);
                                        let advanced_speed_map = fan_map[j].advanced_speed_map.as_ref().unwrap();
                                        let curr_temp = temp;
                                        let mut current_map: Vec<config::AdvancedSpeedMap> = advanced_speed_map.clone();
                                        let mut max_refer: u8 = 0;
                                        current_map.sort();
                                        for speed_map in current_map  {
                                            if speed_map.refer >= max_refer && speed_map.refer <= curr_temp {
                                                max_refer = speed_map.refer;
                                                fan_speed_map = speed_map.speed_map;
                                            }
                                        }
                                        info_log = format!("Detected the advanced speed map of fan {}: {}", i, fan_speed_map);
                                        println!("{}", info_log);
                                        info!("{}", info_log);
                                    }
                                    let fan_temp_map: &Vec<config::TemperaturePoint> = find_temp_fan_speed_map(&data, &fan_speed_map).unwrap();
                                    calc_temp_fan::clear_data();
                                    calc_temp_fan::forset_data(fan_temp_map);
                                    let pwm = calc_temp_fan::calc_fan_pwm(temp);
                                    info_log = format!("Calculate the appropriate fan speed for the fan with ID {} based on CPU chip {}: {}%!", i, name, pwm);
                                    println!("{}", info_log);
                                    info!("{}", info_log);
                                    changed_fan_speed(i, pwm);
                                    calc_temp_fan::clear_data();
                                }
                                if found == false {
                                    let mut info_log = format!("No fan mapping rules configured, execute default temperature mapping!");
                                    println!("{}", info_log);
                                    info!("{}", info_log);
                                    let fan_temp_map: &Vec<config::TemperaturePoint> = find_temp_fan_speed_map(&data, &"default".to_string()).unwrap();
                                    calc_temp_fan::clear_data();
                                    calc_temp_fan::forset_data(fan_temp_map);
                                    let pwm = calc_temp_fan::calc_fan_pwm(temp);
                                    info_log = format!("Calculate the appropriate fan speed for the fan with ID {} based on CPU chip {}: {}%!", i, name, pwm);
                                    println!("{}", info_log);
                                    info!("{}", info_log);
                                    changed_fan_speed(i, pwm);
                                    calc_temp_fan::clear_data();
                                }
                            },
                            None => {
                                let mut info_log = format!("No fan mapping rules configured, execute default temperature mapping!");
                                println!("{}", info_log);
                                info!("{}", info_log);
                                let fan_temp_map: &Vec<config::TemperaturePoint> = find_temp_fan_speed_map(&data, &"default".to_string()).unwrap();
                                calc_temp_fan::clear_data();
                                calc_temp_fan::forset_data(fan_temp_map);
                                let pwm = calc_temp_fan::calc_fan_pwm(temp);
                                info_log = format!("Calculate the appropriate fan speed for the fan with ID {} based on CPU chip {}: {}%!", i, name, pwm);
                                println!("{}", info_log);
                                info!("{}", info_log);
                                changed_fan_speed(i, pwm);
                                calc_temp_fan::clear_data();
                            }
                        }
                    }
                }
            }
        }

        let mut info_log = format!("Check the fan for static mapping!");
        println!("{}", info_log);
        info!("{}", info_log);
        for i in 0..fan_num {
            let fan_map = &data.setting.fan_map;
            match fan_map {
                Some(fan_map) => {
                    let n = fan_map.len();
                    for j in 0..n {
                        let id = fan_map[j].id;
                        if id != i {
                            continue;
                        }
                        if fan_map[j].dynamic_cpu_chip.is_some() || fan_map[j].dynamic_fan_speed_map.is_some() {
                            continue
                        }
                        if fan_map[j].static_fan_map.is_none() {
                            continue;
                        }
                        let pwm = fan_map[j].static_fan_map.unwrap();
                        info_log = format!("Use static speed configuration for Fan {}: {}%!", i, pwm);
                        println!("{}", info_log);
                        info!("{}", info_log);
                        changed_fan_speed(i, pwm);
                    }
                },
                None => {
                }
            }
        }

        sleep(Duration::from_secs(interval));
    }

    let pwm = calc_temp_fan::calc_fan_pwm(45);
    println!("target temp: 45, pwm: {}", pwm);
}
