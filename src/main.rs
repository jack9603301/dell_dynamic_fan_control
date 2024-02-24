extern crate lm_sensors;
extern crate log;
extern crate ipmiraw;

mod config;
mod calc_temp_fan;
use syslog::{Facility, Formatter3164, BasicLogger};
use log::{SetLoggerError, LevelFilter, info};

use std::env;
use ipmiraw::si::Ipmi;
use std::thread::sleep;
use std::time::Duration;
use std::process::Command;

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
    calc_temp_fan::forset_data(&data);

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
    log::set_boxed_logger(Box::new(BasicLogger::new(logger)))
            .map(|()| log::set_max_level(LevelFilter::Info));

    let fan_num = data.setting.fan_num;
    let interval = data.setting.interval;

    println!("Detected {} fans!", fan_num);
    info!("Detected {} fans!", fan_num);

    println!("Perform detection every {} seconds!", interval);
    info!("Perform detection every {} seconds!", interval);

    while true {
        let mut coretemp_sum: f64 = 0.0;
        let mut count_temp: u8 = 0;
        for chip in sensors.chip_iter(None) {
            if let Some(path) = chip.path() {
                let name: String = chip.name().expect("Failed to get chip name!");
                if name.contains("coretemp") {
                    println!("CPU Chip: Checking, {}!",chip);
                    for feature in chip.feature_iter() {
                        let name = feature.name().transpose().unwrap().unwrap_or("N/A");
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
                                    coretemp_sum = coretemp_sum + value.raw_value();
                                    count_temp = count_temp + 1;
                                }
                            }
                        }
                    }
                }
            }
        }
        let temp = (coretemp_sum / count_temp as f64) as u8;
        let mut info_log = format!("Average temperature detected: {}!", temp);
        println!("{}", info_log);
        info!("{}", info_log);
        let pwm = calc_temp_fan::calc_fan_pwm(temp);
        info_log = format!("Calculate the appropriate fan speed: {}%!", pwm);
        println!("{}", info_log);
        info!("{}", info_log);
        
        for i in 0..fan_num {
            changed_fan_speed(i, pwm);
        }

        sleep(Duration::from_secs(interval));
    }

    let pwm = calc_temp_fan::calc_fan_pwm(45);
    println!("target temp: 45, pwm: {}", pwm);
}
