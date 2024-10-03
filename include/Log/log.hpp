#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

enum LogLevel
{
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Log
{
public:
    Log(const std::string &filename, LogLevel level = INFO) : m_filename(filename), m_level(level)
    {
        m_file.open(filename, std::ios::trunc | std::ios::out);
    }
    ~Log()
    {
        m_file.close();
    }
    void write(LogLevel level, const std::string &message)
    {
        if (level >= m_level)
        {
            std::string time = getFormattedTime();

            m_file << time << " [" << levelToString(level) << "] " << message << std::endl;
        }
    }

private:
    std::string m_filename;
    LogLevel m_level;
    std::fstream m_file;

    std::string levelToString(LogLevel level)
    {
        switch (level)
        {
        case DEBUG:
            return "DEBUG";
        case INFO:
            return "INFO";
        case WARNING:
            return "WARNING";
        case ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
        }
    }
    std::string getFormattedTime(const std::string &format = "%Y-%m-%d %H:%M:%S")
    {
        time_t now = time(0);
        char strftime_buf[sizeof("YYYY-MM-DD HH:MM:SS")];
        strftime(strftime_buf, sizeof(strftime_buf), format.c_str(), localtime(&now));
        return std::string(strftime_buf);
    }
};

#endif // LOG_H
