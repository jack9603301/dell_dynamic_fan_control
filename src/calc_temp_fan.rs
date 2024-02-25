
use std::vec::Vec;

static mut g_temperatures: Vec<u8> = Vec::new();
static mut g_fan_pwms: Vec<u8> = Vec::new();

fn line_inter(x0: u8, y0: u8, x1: u8, y1: u8, x: u8) -> u8 {
    if x0 == x1 {
        return y0;
    }

    let slope = (y1 as f64 - y0 as f64) / (x1 as f64 - x0 as f64);

    let intercept = y0 as f64 - slope * x0 as f64;

    let target = slope * x as f64 + intercept;

    return target as u8;
}

fn inter(temperatures: &Vec<u8>, pwms: &Vec<u8>, target: u8) -> u8 {
    let mut prev_temp = temperatures[0];
    let mut prev_pwm = pwms[0];
    if target <= prev_temp {
        return prev_pwm;
    }
    let n = temperatures.len();
    for i in 1..n {
        if target <= temperatures[i] {
            println!("{}", temperatures[i]);
            return line_inter(prev_temp, prev_pwm, temperatures[i], pwms[i], target);
        }
        prev_temp = temperatures[i];
        prev_pwm = pwms[i];
    }
    return prev_pwm;
}

pub fn clear_data() {
    unsafe {
        g_temperatures.clear();
        g_fan_pwms.clear();
    }
}

pub fn forset_data(points: & Vec<super::config::TemperaturePoint>) {
    for point in points {
        println!("Temperature: {}, FAN Speed: {}", point.temperature, point.fan_speed);
        unsafe {
            g_temperatures.push(point.temperature);
            g_fan_pwms.push(point.fan_speed);
        }
    }
}

pub fn calc_fan_pwm(temperature: u8) -> u8 {
    unsafe {
        return inter(&g_temperatures, &g_fan_pwms, temperature);
    }
}
