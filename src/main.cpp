#include "log.hpp"
#include "power.hpp"
#include "config.hpp"
#include "activity.hpp"
#include <thread>
#include <chrono>
#include <unordered_map>
enum Bypass_Event
{
    None,
    Always,
    Temp,
    Capacity,
    Game
};
struct Charge_Info
{
    bool Bypass_Status = false;
    bool FastCharge_Status = false;
    Bypass_Event Event = Bypass_Event::None;
};
/// @brief 
struct Bypass_Config {
    bool Bypass_Status = false;
    BypassMode Mode = BypassMode::Current;//旁路模式
    int Check_WaitTime = 30;
    int Input_WaitTime = 5;
    int Bypass_Current = 0;
    bool Bypass_Always = false;//一直开启旁路供电
    bool Bypass_Temp_Status = false;//温度旁路供电
    bool Bypass_Capacity_Status = false;//电量旁路供电
    float Bypass_Temp_Close = 0;//旁路供电关闭温度
    float Bypass_Temp_Open = 0;
    int Bypass_Capacity_Close = 0;//旁路供电关闭电量
    int Bypass_Capacity_Open = 0;
    bool FastCharge_Status = false;//闲时快充
    int FastCharge_Current = 0;//闲时快充电流
    int FastCharge_CloseCapacity = 0;//闲时快充关闭电量
    int FastCharge_CloseTemp = 0;//闲时快充关闭温度
    bool Battery_Info = false;//显示电池信息
    int Battery_Info_WaitTime = 120;
    std::map<std::string, std::string> Game_List;//游戏列表
};
Log logger("/sdcard/test.txt", LogLevel::INFO);
Config config_tool;
power Power_tool(logger);
Bypass_Config Bypass_config;
Charge_Info Info;
Activity activity_tool(logger);
void lock()
{
    bool bypass = false;
    bool fastcharger = false;
    BypassMode Mode;
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (bypass && !Info.Bypass_Status)
        {
            bypass = false;
            Power_tool.Bypass_Charger(false, Mode);
        }
        else if (fastcharger && !Info.FastCharge_Status)
        {
            fastcharger = false;
            Power_tool.Bypass_Charger(false, BypassMode::Current); // 恢复电流
        }
        if (Info.Bypass_Status)
        {
            bypass = true;
            Mode = Bypass_config.Mode;
            Power_tool.Bypass_Charger(true, Bypass_config.Mode, Bypass_config.Input_WaitTime, Bypass_config.Bypass_Current); // 锁定旁路
        }
        else if (Info.FastCharge_Status)
        {
            fastcharger = true;
            Power_tool.edit_current(Bypass_config.FastCharge_Current, Bypass_config.FastCharge_Current); // 锁定睡眠
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
}

void info()
{
    auto to_string = [](BatteryStatus status) -> std::string
        {
            static const std::unordered_map<BatteryStatus, std::string> status_map = {
                {BatteryStatus::Unknown, "Unknown"},
                {BatteryStatus::Charging, "Charging"},
                {BatteryStatus::Discharging, "Discharging"},
                {BatteryStatus::Full, "Full"}
            };
            auto it = status_map.find(status);
            if (it != status_map.end()) {
                return it->second;
            }
            return "Unknown";
        };
    while (true)
    {
        float Temp = Power_tool.get_Temp();
        int Capacity = Power_tool.get_Capacity();
        double Power = Power_tool.get_Power();
        BatteryStatus Status = Power_tool.get_Status();
        logger.write(LogLevel::INFO, "电池温度:" + std::to_string(Temp) + "电池电量:" + std::to_string(Capacity) + "当前功耗:" + std::to_string(-Power) + "电池状态:" + to_string(Status));
        std::this_thread::sleep_for(std::chrono::seconds(Bypass_config.Battery_Info_WaitTime));
    }
}
int main()
{
    std::thread lock_t(lock);
    lock_t.detach();
    std::thread info_t(info);
    info_t.detach();
    logger.write(LogLevel::INFO, "旁路供电模块加载完成");
    float Temp;
    int Capacity;
    double Power;
    BatteryStatus Status;
    while (true)
    {
        auto config = config_tool.get_config();
        std::this_thread::sleep_for(std::chrono::seconds(Bypass_config.Check_WaitTime));
        try {
            Temp = Power_tool.get_Temp();
            Capacity = Power_tool.get_Capacity();
            Power = Power_tool.get_Power();
            Status = Power_tool.get_Status();
        }
        catch (const std::exception& e) {
            logger.write(LogLevel::ERROR, "Read battery information failed");
            continue;
        }
        try {
            {
                Bypass_config.Bypass_Status = config["总开关"]["开启旁路供电"] == "1";
                Bypass_config.Mode = (BypassMode)std::stoi(config["总开关"]["旁路模式"]);
                Bypass_config.Input_WaitTime = std::stoi(config["总开关"]["插拔间隔"]);
                Bypass_config.Check_WaitTime = std::stoi(config["总开关"]["间隔时间"]);
                Bypass_config.Bypass_Current = std::stoi(config["总开关"]["旁路电流"]);
                Bypass_config.Bypass_Always = config["总开关"]["一直开启旁路供电"] == "1";
            }//总开关
            {
                Bypass_config.Bypass_Capacity_Status = config["电池电量"]["开关状态"] == "1";
                Bypass_config.Bypass_Capacity_Close = std::stoi(config["电池电量"]["关闭电量"]);
                Bypass_config.Bypass_Capacity_Open = std::stoi(config["电池电量"]["开启电量"]);
            }//电池电量
            {
                Bypass_config.Bypass_Temp_Status = config["电池温度"]["开关状态"] == "1";
                Bypass_config.Bypass_Temp_Open = std::stof(config["电池温度"]["开启温度"]);
                Bypass_config.Bypass_Temp_Close = std::stof(config["电池温度"]["关闭温度"]);
            }//电池温度
            {
                Bypass_config.FastCharge_Status = config["闲时快充"]["开关状态"] == "1";
                Bypass_config.FastCharge_Current = std::stoi(config["闲时快充"]["快充电流"]);
                Bypass_config.FastCharge_CloseCapacity = std::stoi(config["闲时快充"]["关闭电量"]);
                Bypass_config.FastCharge_CloseTemp = std::stof(config["闲时快充"]["关闭温度"]);
            }//闲时快充
            {
                Bypass_config.Battery_Info = config["电池信息"]["间隔时间"] == "1";
                Bypass_config.Battery_Info_WaitTime = std::stoi(config["电池信息"]["间隔时间"]);
            }//电池信息
            {
                Bypass_config.Game_List = config["游戏"];
            }//游戏
        }
        catch (const std::exception& e) {
            logger.write(LogLevel::ERROR, "Can't read config:" + std::string(e.what()));
            continue;
        }//从配置文件中读取配置
        if (Bypass_config.Bypass_Status)
        {
            if (Bypass_config.Bypass_Always && !Info.Bypass_Status)
            {
                Info.Bypass_Status = true;
                Info.Event = Bypass_Event::Always;
                logger.write(LogLevel::INFO, "一直开启旁路供电开启");
            }
            else if (!Bypass_config.Bypass_Always && Info.Event == Bypass_Event::Always)
            {
                Info.Event = Bypass_Event::None;
                Info.Bypass_Status = false;
                logger.write(LogLevel::INFO, "一直开启旁路供电关闭");
            }
            else if (Info.Event == Bypass_Event::Always)
            {
                continue;
            }
        }
        if (Status == BatteryStatus::Charging)
        {
            if (!Info.Bypass_Status || Info.Event == Bypass_Event::Game)
            {
                bool findgame = false;
                for (const auto& pair : Bypass_config.Game_List)
                {
                    if (activity_tool.getForegroundAppPackageName() == pair.second)
                    {
                        findgame = true;
                        logger.write(LogLevel::INFO, "找到游戏:" + pair.second);
                        break;
                    }
                }
                if (findgame)
                {
                    Info.Event = Bypass_Event::Game;
                    Info.Bypass_Status = true;
                    logger.write(LogLevel::INFO, "游戏旁路供电开启");
                    continue;
                }
                else if (Info.Bypass_Status && Info.Event == Bypass_Event::Game)
                {
                    Info.Event = Bypass_Event::None;
                    Info.Bypass_Status = false;
                    logger.write(LogLevel::INFO, "游戏旁路供电关闭");
                }
            }
            if (Bypass_config.Bypass_Temp_Status)
            {
                if (Info.Event == Bypass_Event::None && Temp >= Bypass_config.Bypass_Temp_Open)
                {
                    if (Info.FastCharge_Status)
                    {
                        Info.FastCharge_Status = false;
                        logger.write(LogLevel::INFO, "温度旁路供电开启,闲时快充关闭");
                    }else{
                        logger.write(LogLevel::INFO, "温度旁路供电开启");
                    }
                    Info.Event = Bypass_Event::Capacity;
                    Info.Bypass_Status = true;
                    continue;
                }
                else if (Info.Event == Bypass_Event::Temp && Temp < Bypass_config.Bypass_Temp_Close)
                {
                    Info.Event = Bypass_Event::None;
                    Info.Bypass_Status = false;
                    logger.write(LogLevel::INFO, "温度旁路供电关闭");
                }
            }
            else if (!Bypass_config.Bypass_Capacity_Status && Info.Event == Bypass_Event::Temp)
            {
                Info.Event = Bypass_Event::None;
                Info.Bypass_Status = false;
                logger.write(LogLevel::INFO, "温度旁路供电关闭");
            }
            if (config["电池电量"]["总开关"] == "1")
            {
                if (Info.Event == Bypass_Event::None && Capacity >= Bypass_config.Bypass_Capacity_Open)
                {
                    if (Info.FastCharge_Status)
                    {
                        Info.FastCharge_Status = false;
                        logger.write(LogLevel::INFO, "电量旁路供电开启,闲时快充关闭");
                    }else{
                        logger.write(LogLevel::INFO, "电量旁路供电开启");
                    }
                    Info.Event = Bypass_Event::Capacity;
                    Info.Bypass_Status = true;
                    continue;
                }
                else if (Info.Event == Bypass_Event::Capacity && Capacity < Bypass_config.Bypass_Capacity_Close)
                {
                    Info.Event = Bypass_Event::None;
                    Info.Bypass_Status = false;
                    logger.write(LogLevel::INFO, "电量旁路供电关闭");
                }
            }
            else if (!Bypass_config.Bypass_Capacity_Status && Info.Event == Bypass_Event::Capacity)
            {
                Info.Event = Bypass_Event::None;
                Info.Bypass_Status = false;
                logger.write(LogLevel::INFO, "电量旁路供电关闭");
            }
            if (Bypass_config.FastCharge_Status)
            {
                if (!Info.Bypass_Status && !Info.FastCharge_Status && Capacity < Bypass_config.FastCharge_CloseCapacity && Temp < Bypass_config.FastCharge_CloseTemp)
                {
                    Info.FastCharge_Status = true;
                    logger.write(LogLevel::INFO, "闲时快充开启");
                }
                else if (Info.FastCharge_Status && ((Capacity >= Bypass_config.FastCharge_CloseCapacity) || (Temp >= Bypass_config.FastCharge_CloseTemp)))

                {
                    Info.FastCharge_Status = false;
                    logger.write(LogLevel::INFO, "闲时快充关闭");
                }
            }
            else if (Info.FastCharge_Status && !Bypass_config.FastCharge_Status)
            {
                Info.FastCharge_Status = false;
                logger.write(LogLevel::INFO, "闲时快充关闭");
            }
        }
        else if (Status != BatteryStatus::Charging && (Info.Bypass_Status || Info.FastCharge_Status))
        {
            Info.Event = Bypass_Event::None;
            Info.Bypass_Status = false;
            Info.FastCharge_Status = false;
            logger.write(LogLevel::INFO, "不处于充电状态,自动关闭功能");
        }
        else if (Status != BatteryStatus::Charging && (Info.Bypass_Status || Info.FastCharge_Status))
        {
            Info.Event = Bypass_Event::None;
            Info.Bypass_Status = false;
            Info.FastCharge_Status = false;
            logger.write(LogLevel::INFO, "不处于充电状态,自动关闭功能");
        }
    }
    return 0;
}
