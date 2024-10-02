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
    std::unique_ptr<File> NightCharge_file = std::make_unique<File>("/sys/class/power_supply/battery/night_charging", logger);
    std::unique_ptr<File> ReCharge_file = std::make_unique<File>("/sys/class/power_supply/battery/force_recharge", logger);
    std::unique_ptr<File> Input_file = std::make_unique<File>("/sys/class/power_supply/battery/input_suspend", logger);
    std::unique_ptr<File> FastCharge_file = std::make_unique<File>("/sys/class/power_supply/battery/fast_charge_current", logger);
    std::unique_ptr<File> ThermalCharge_file = std::make_unique<File>("/sys/class/power_supply/battery/thermal_input_current", logger);
    std::unique_ptr<File> Capacity_file = std::make_unique<File>("/sys/class/power_supply/battery/capacity", logger);
    std::unique_ptr<File> Temp_file = std::make_unique<File>("/sys/class/power_supply/battery/temp", logger);
    std::unique_ptr<File> Voltage_file = std::make_unique<File>("/sys/class/power_supply/battery/voltage_now", logger);
    std::unique_ptr<File> Current_file = std::make_unique<File>("/sys/class/power_supply/battery/current_now", logger);
    std::unique_ptr<File> Status_file = std::make_unique<File>("/sys/class/power_supply/battery/status", logger);

public:
    power(Log &logger) : logger(logger)
    {
        std::string buffer;
        std::string buffer1;
        FastCharge_file->read_all(&buffer);
        logger.write(LogLevel::INFO, "Origin_FastCharge_Current: " + buffer);
        ThermalCharge_file->read_all(&buffer1);
        logger.write(LogLevel::INFO, "Origin_ThermalCharge_Current: " + buffer1);
        Origin_FastCharge_Current = std::stoi(buffer);
        Origin_ThermalCharge_Current = std::stoi(buffer1);
    }
    int get_Capacity()
    {
        return Capacity_file->get_value<int>();
    }

    float get_Temp()
    {
        return Temp_file->get_value<float>();
    }

    double get_Voltage()
    {
        return Voltage_file->get_value<double>();
    }

    double get_Current()
    {
        return Current_file->get_value<double>();
    }
    double get_Power()
    {
        double voltage = get_Voltage();
        double current = get_Current();
        return voltage * current;
    }
    bool edit_current(int FastChargeCurrent, int ThermalChargeCurrent)
    {
        FastCharge_file->trunc_write(std::to_string(FastChargeCurrent));
        ThermalCharge_file->trunc_write(std::to_string(ThermalChargeCurrent));
        return true;
    }
    enum BatteryStatus get_Status()
    {
        std::string buffer;
        Status_file->read_all(&buffer);
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
                NightCharge_file->trunc_write("1");
                ReCharge_file->trunc_write("1");
                std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
                Input_file->trunc_write("1");
            }
            else
            {
                NightCharge_file->trunc_write("0");
                ReCharge_file->trunc_write("0");
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
