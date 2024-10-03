#include "log.hpp"
#include "power.hpp"
#include "config.hpp"
#include "activity.hpp"
#include <thread>
#include <chrono>
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
    bool Battery_Info = false;
    bool Bypass_Status = false;
    bool FastCharge_Status = false;
    int FastCharge_Current;
    int Bypass_Current;
    int Input_WaitTime = 5;
    int Battery_Info_WaitTime = 120;
    Bypass_Event Event = Bypass_Event::None;
    BypassMode Mode;
};

Log logger("/sdcard/test.txt", LogLevel::INFO);
Config config_tool;
power Power_tool(logger);
Charge_Info Info;
Activity activity_tool(logger);
void lock()
{
    bool bypass = false;
    bool fastcharger = false;
    BypassMode Mode;
    while (true)
    {
        // std::this_thread::sleep_for(std::chrono::seconds(1));
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
            Mode = Info.Mode;
            Power_tool.Bypass_Charger(true, Info.Mode, Info.Input_WaitTime, Info.Bypass_Current); // 恢复电流
        }
        else if (Info.FastCharge_Status)
        {
            fastcharger = true;
            Power_tool.edit_current(Info.FastCharge_Current, Info.FastCharge_Current);
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
}
void info()
{
    while (true)
    {
        float Temp = Power_tool.get_Temp();
        int Capacity = Power_tool.get_Capacity();
        double Power = Power_tool.get_Power();
        logger.write(LogLevel::INFO, "电池温度:" + std::to_string(Temp) + "电池电量:" + std::to_string(Capacity) + "当前功耗:" + std::to_string(-Power));
        std::this_thread::sleep_for(std::chrono::seconds(Info.Battery_Info_WaitTime));
    }
}
int main()
{
    std::thread lock_t(lock);
    lock_t.detach();
    std::thread info_t(info);
    info_t.detach();
    logger.write(LogLevel::INFO, "旁路供电模块加载完成");
    while (true)
    {
        auto config = config_tool.get_config();
        std::this_thread::sleep_for(std::chrono::seconds(std::stoi(config["总开关"]["间隔时间"])));
        float Temp = Power_tool.get_Temp();
        int Capacity = Power_tool.get_Capacity();
        double Power = Power_tool.get_Power();
        BatteryStatus Status = Power_tool.get_Status();
        if (config["总开关"]["旁路模式"] == "0")
        {
            Info.Mode = BypassMode::Current; // 电流模式
        }
        else if (config["总开关"]["旁路模式"] == "1")
        {
            Info.Mode = BypassMode::Night; // 夜间模式
        }
        else
        {
            logger.write(LogLevel::ERROR, "未知旁路模式:" + config["总开关"]["旁路模式"]);
            continue;
        }
        if (config["电池信息"]["总开关"] == "1")
        {
            Info.Battery_Info = true;
            Info.Battery_Info_WaitTime = std::stoi(config["电池信息"]["间隔时间"]);
        }
        else if (config["电池信息"]["总开关"] == "0")
        {
            Info.Battery_Info = false;
        }
        else
        {
            logger.write(LogLevel::ERROR, "未知电池信息选项:" + config["电池信息"]["总开关"]);
        }
        int FastChargeCurrent = std::stoi(config["闲时快充"]["快充电流"]);
        Info.FastCharge_Current = FastChargeCurrent;
        int BypassCurrent = std::stoi(config["总开关"]["电流"]);
        Info.Bypass_Current = BypassCurrent;
        int InputWaitTime = std::stoi(config["总开关"]["插拔间隔"]);
        Info.Input_WaitTime = InputWaitTime;
        if (config["总开关"]["开启旁路供电"] == "1")
        {
            if (config["总开关"]["一直开启旁路供电"] == "1" && !Info.Bypass_Status)
            {
                Info.Bypass_Status = true;
                Info.Event = Bypass_Event::Always;
                logger.write(LogLevel::INFO, "一直开启旁路供电开启");
            }
            else if (config["总开关"]["一直开启旁路供电"] == "0" && Info.Event == Bypass_Event::Always)
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
                for (const auto &pair : config.find("游戏")->second)
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
                }
                else if (Info.Bypass_Status && Info.Event == Bypass_Event::Game)
                {
                    Info.Event = Bypass_Event::None;
                    Info.Bypass_Status = false;
                    logger.write(LogLevel::INFO, "游戏旁路供电关闭");
                }
            }
            if (config["电池温度"]["总开关"] == "1")
            {
                if (Info.Event == Bypass_Event::None && Temp >= std::stof(config["电池温度"]["开启温度"]))
                {
                    if (Info.FastCharge_Status)
                    {
                        Info.FastCharge_Status = false;
                        logger.write(LogLevel::INFO, "温度旁路供电开启,闲时快充关闭");
                    }
                    Info.Event = Bypass_Event::Capacity;
                    Info.Bypass_Status = false;
                    continue;
                }
                else if (Info.Event == Bypass_Event::Temp && Temp < std::stof(config["电池温度"]["关闭温度"]))
                {
                    Info.Event = Bypass_Event::None;
                    Info.Bypass_Status = false;
                    logger.write(LogLevel::INFO, "温度旁路供电关闭");
                }
            }
            else if (config["电池温度"]["总开关"] == "0" && Info.Event == Bypass_Event::Temp)
            {
                Info.Event = Bypass_Event::None;
                Info.Bypass_Status = false;
                logger.write(LogLevel::INFO, "温度旁路供电关闭");
            }
            if (config["电池电量"]["总开关"] == "1")
            {
                if (Info.Event == Bypass_Event::None && Capacity >= std::stoi(config["电池电量"]["开启电量"]))
                {
                    if (Info.FastCharge_Status)
                    {
                        Info.FastCharge_Status = false;
                        logger.write(LogLevel::INFO, "电量旁路供电开启,闲时快充关闭");
                    }
                    Info.Event = Bypass_Event::Capacity;
                    Info.Bypass_Status = true;
                    continue;
                }
                else if (Info.Event == Bypass_Event::Capacity && Capacity < std::stoi(config["电池温度"]["关闭电量"]))
                {
                    Info.Event = Bypass_Event::None;
                    Info.Bypass_Status = false;
                    logger.write(LogLevel::INFO, "电量旁路供电关闭");
                }
            }
            else if (config["电池温度"]["总开关"] == "0" && Info.Event == Bypass_Event::Capacity)
            {
                Info.Event = Bypass_Event::None;
                Info.Bypass_Status = false;
                logger.write(LogLevel::INFO, "温度旁路供电关闭");
            }
            if (config["闲时快充"]["总开关"] == "1")
            {
                if (!Info.Bypass_Status && !Info.FastCharge_Status && Capacity < std::stoi(config["闲时快充"]["关闭电量"]) && Temp < std::stof(config["闲时快充"]["关闭温度"]))
                {
                    Info.FastCharge_Status = true;
                    logger.write(LogLevel::INFO, "闲时快充开启");
                }
                else if (Info.FastCharge_Status && (Capacity >= std::stoi(config["闲时快充"]["关闭电量"]) || Temp >= std::stof(config["闲时快充"]["关闭温度"])))
                {
                    Info.FastCharge_Status = false;
                    logger.write(LogLevel::INFO, "闲时快充关闭");
                }
            }
            else if (Info.FastCharge_Status && config["闲时快充"]["总开关"] == "0")
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
    }
    return 0;
}
