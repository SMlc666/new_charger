#pragma once
#include <array>
#include <cstdio>
#include <memory>
#include <regex>
#include <string>
class Activity
{
private:
  Log& logger;
  std::string execCommand(const char* cmd)
  {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
    {
      logger.write(LogLevel::ERROR, "popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
      result += buffer.data();
    }
    return result;
  }

public:
  Activity(Log& logger) : logger(logger) {}
  bool isAppRunning(const std::string& packageName) {
    std::string cmd = "dumpsys activity top | grep -E 'ACTIVITY|mResumedActivity'";
    std::string output = execCommand(cmd.c_str());
    std::istringstream iss(output);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.find(packageName) != std::string::npos) {
            return true;
        }
    }
    return false;
}
};
