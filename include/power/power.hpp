#pragma once
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <sstream>
enum BypassMode{
    Night,//夜间供电模式
    Current//电流模式
};
enum BatteryStatus{
    Unknown,//电池状态未知。
    Charging,//电池正在充电。
    Discharging,//电池正在放电
    Full//电池充满电。
};
class power{
    private:
        Log &logger;
        bool enable_rw(const std::string filename){
            mode_t m_mode = 0777;
            if (chmod(filename.c_str(),m_mode) == -1){
                logger.write(LogLevel::ERROR,filename + "Cannot chmod to make it modifiable");
                return false;
            }
            return true;
        }
        bool open_file(std::fstream &file,const std::string filename){
            enable_rw(filename);
            file.open(filename,std::ios::out | std::ios::in);
            if (!file.is_open()){
                logger.write(LogLevel::ERROR,filename + " Could not open it");
                return false;
            }
            return true;
        }
        bool trunc_write(std::fstream &file,const std::string filename,const std::string content){
            if (!file.is_open()){
                logger.write(LogLevel::ERROR,"When write the contents of the file, the file is not open"); 
                return false;
            }
            truncate(filename.c_str(),0);
            file << content;
            return true;
        }

        bool read_all(std::fstream &file, std::string *buffer) {
            if (file.is_open()){
                logger.write(LogLevel::ERROR,"When reading the contents of the file, the file is not open");
                return false;
            }
            std::stringstream ss;
            ss << file.rdbuf(); // 读取整个文件到字符串流
            *buffer = ss.str(); // 将字符串流的内容转移到buffer中
        
            return true; // 读取成功
        }
        template <typename T>
        T get_value_from_file(std::fstream& file) {
            std::string buffer;
            read_all(file, &buffer); // 读取文件内容
        
            T value; // 目标类型变量
            std::istringstream iss(buffer); // 将字符串转换为输入流
        
            // 试图从输入流中提取类型 T 的值
            if (!(iss >> value)) {
                // 如果转换失败，抛出 invalid_argument 异常
                logger.write(LogLevel::ERROR,"Failed to convert file content to the specified type.") ;
            }
        
            // 检查是否有剩余的非空白字符，如果有则表明转换不完全
            if (iss && iss.peek() != EOF && !isspace(iss.peek())) {
                logger.write(LogLevel::ERROR,"File contains extra data after value.");
            }
        
            return value; // 返回转换后的值
        }
        int Origin_FastCharge_Current;
        int Origin_ThermalCharge_Current;
        std::string NightCharge_filename = "/sys/class/power_supply/battery/night_charging";//夜间充电文件目录
        std::fstream NightCharge_file;//夜间充电文件流
        std::string ReCharge_filename = "/sys/class/power_supply/battery/force_recharge";//回充文件目录
        std::fstream ReCharge_file;//回充文件流
        std::string Input_filename = "/sys/class/power_supply/battery/input_suspend";//充电文件目录
        std::fstream Input_file;//充电文件流
        std::string FastCharge_filename = "/sys/class/power_supply/battery/fast_charge_current";
        std::fstream FastCharge_file;
        std::string ThermalCharge_filename = "/sys/class/power_supply/battery/thermal_input_current";
        std::fstream ThermalCharge_file;
        std::string Capacity_filename = "/sys/class/power_supply/battery/capacity";//电量文件目录
        std::fstream Capacity_file;//电量文件流
        std::string Temp_filename = "/sys/class/power_supply/battery/temp";//电池温度文件目录
        std::fstream Temp_file;//电池温度文件流
        std::string Voltage_filename = "/sys/class/power_supply/battery/voltage_now";//电压文件目录
        std::fstream Voltage_file;//电压文件流
        std::string Current_filename = "/sys/class/power_supply/battery/current_now";//电流文件目录
        std::fstream Current_file;//电流文件流
        std::string Status_filename = "/sys/class/power_supply/battery/status";//电池状态文件目录
        std::fstream Status_file;//电池状态文件流
        
    public:
        int get_Capacity() {
            return get_value_from_file<int>(Capacity_file);
        }
        
        float get_Temp() {
            return get_value_from_file<float>(Temp_file);
        }
        
        double get_Voltage() {
            return get_value_from_file<double>(Voltage_file);
        }
        
        double get_Current() {
            return get_value_from_file<double>(Current_file);
        }
        double get_Power(){
            double voltage = get_Voltage();
            double current = get_Current();
            return voltage * current;
        }
        bool edit_current(int FastChargeCurrent,int ThermalChargeCurrent){
            trunc_write(FastCharge_file,FastCharge_filename,std::to_string(FastChargeCurrent));
            trunc_write(ThermalCharge_file,ThermalCharge_filename,std::to_string(ThermalChargeCurrent));
            return true;
        }
        enum BatteryStatus get_Status(){
            std::string buffer;
            read_all(Status_file,&buffer);
            if (buffer == "Charging"){
                return Charging;
            }else if(buffer == "Discharging"){
                return Discharging;
            }else if(buffer == "Full"){
                return Full;
            }else{
                return Unknown;
            }
        }
        bool Bypass_Charger(bool status,BypassMode mode,int wait_time = 5000,int Current = 500){
            if (mode == BypassMode::Night){
                if (status){
                    trunc_write(NightCharge_file,NightCharge_filename,"1");
                    trunc_write(ReCharge_file,ReCharge_filename,"1");
                    std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
                    trunc_write(Input_file,Input_filename,"1");
                }else{
                    trunc_write(NightCharge_file,NightCharge_filename,"0");
                    trunc_write(ReCharge_file,ReCharge_filename,"0");
                    trunc_write(Input_file,Input_filename,"0");
                }
            }else if(mode == BypassMode::Current){
                if (status){
                    edit_current(Current,Current);
                }else{
                    edit_current(Origin_FastCharge_Current,Origin_ThermalCharge_Current);
                }
            }
            return true;
        }
        
        power(Log &logger) : logger(logger){
            open_file(NightCharge_file,NightCharge_filename);
            open_file(ReCharge_file,ReCharge_filename);
            open_file(Input_file,Input_filename);
            open_file(FastCharge_file,FastCharge_filename);
            open_file(ThermalCharge_file,ThermalCharge_filename);
            open_file(Capacity_file,Capacity_filename);
            open_file(Temp_file,Temp_filename);
            open_file(Voltage_file,Voltage_filename);
            open_file(Current_file,Current_filename);
            open_file(Status_file,Status_filename);
            std::string buffer;
            std::string buffer1;
            read_all(FastCharge_file,&buffer);
            read_all(ThermalCharge_file,&buffer1);
            Origin_FastCharge_Current = std::stoi(buffer);
            Origin_ThermalCharge_Current = std::stoi(buffer1);
        }
        ~power(){
            NightCharge_file.close();
            ReCharge_file.close();
            Input_file.close();
            FastCharge_file.close();
            ThermalCharge_file.close();
            Capacity_file.close();
            Temp_file.close();
            Voltage_file.close();
            Current_file.close();
            Status_file.close();
        }
};
