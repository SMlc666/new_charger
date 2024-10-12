#pragma once
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <sstream>
#include "file.hpp"
enum BypassMode
{
    Night,  // 夜间供电模式
    Current // 电流模式
};
enum BatteryStatus
{
    Unknown,     // 电池状态未知。
    Charging,    // 电池正在充电。
    Discharging, // 电池正在放电
    Full         // 电池充满电。
};
class power
{
private:
    Log &logger;
    int Origin_FastCharge_Current;
    int Origin_ThermalCharge_Current;
    std::unique_ptr<File> Input_file = std::make_unique<File>("/sys/class/power_supply/battery/input_suspend", logger);
    std::unique_ptr<File> FastCharge_file = std::make_unique<File>("/sys/class/power_supply/battery/fast_charge_current", logger);
    std::unique_ptr<File> ThermalCharge_file = std::make_unique<File>("/sys/class/power_supply/battery/thermal_input_current", logger);
    std::unique_ptr<File> Capacity_file = std::make_unique<File>("/sys/class/power_supply/battery/capacity", logger);
    std::unique_ptr<File> Temp_file = std::make_unique<File>("/sys/class/power_supply/battery/temp", logger);
    std::unique_ptr<File> Voltage_file = std::make_unique<File>("/sys/class/power_supply/battery/voltage_now", logger);
    std::unique_ptr<File> Current_file = std::make_unique<File>("/sys/class/power_supply/battery/current_now", logger);
    std::unique_ptr<File> Status_file = std::make_unique<File>("/sys/class/power_supply/battery/status", logger);
    std::unique_ptr<File> Present_file = std::make_unique<File>("/sys/class/power_supply/usb/present", logger);
public:
    power(Log &logger) : logger(logger)
    {
        std::string buffer;
        std::string buffer1;
        FastCharge_file->read_Line(&buffer);
        ThermalCharge_file->read_Line(&buffer1);
        Origin_FastCharge_Current = std::stoi(buffer);
        Origin_ThermalCharge_Current = std::stoi(buffer1);
    }
    int get_Capacity()
    {
        return Capacity_file->get_value<int>();
    }

    float get_Temp()
    {
        return Temp_file->get_value<float>() / 10;
    }

    double get_Voltage()
    {
        return Voltage_file->get_value<double>();
    } // 微伏

    double get_Current()
    {
        return Current_file->get_value<double>();
    } // 微安
    double get_Power()
    {
        double voltage = get_Voltage() * 1e-6;
        double current = get_Current() * 1e-6;
        return (voltage * current);
    } // 瓦
    bool have_Usb()
    {
        return Present_file->get_value<bool>();
    }//获取USB插入状态
    bool edit_current(int FastChargeCurrent, int ThermalChargeCurrent)
    {
        FastCharge_file->trunc_write(std::to_string(FastChargeCurrent));
        ThermalCharge_file->trunc_write(std::to_string(ThermalChargeCurrent));
        return true;
    }
    enum BatteryStatus get_Status()
    {
        std::string buffer;
        Status_file->read_Line(&buffer);
        if (buffer == "Charging")
        {
            return Charging;
        }
        else if (buffer == "Discharging")
        {
            return Discharging;
        }
        else if (buffer == "Full")
        {
            return Full;
        }
        else
        {
            return Unknown;
        }
    }
    bool Bypass_Charger(bool status, BypassMode mode, int wait_time = 5000, int Current = 500)
    {
        if (mode == BypassMode::Night)
        {
            if (status)
            {
                Input_file->trunc_write("1");
            }
            else
            {
                Input_file->trunc_write("0");
            }
        }
        else if (mode == BypassMode::Current)
        {
            if (status)
            {
                edit_current(Current, Current);
            }
            else
            {
                edit_current(Origin_FastCharge_Current, Origin_ThermalCharge_Current);
            }
        }
        return true;
    }
};
