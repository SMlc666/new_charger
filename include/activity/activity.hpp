#pragma once
#include <array>
#include <cstdio>
#include <memory>
#include <regex>
#include <string>
class Activity
{
private:
  Log &logger;
  std::string execCommand(const char *cmd)
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
  Activity(Log &logger) : logger(logger) {}
  std::string getForegroundAppPackageName()
  {
    const char *cmd = "dumpsys window | grep 'mCurrentFocus'";
    std::string line;
    try
    {
      line = execCommand(cmd);
    }
    catch (const std::exception &e)
    {
      // Log error: Command execution failed
      logger.write(LogLevel::ERROR, "Error executing command");
      return "";
    }

    if (line.empty())
    {
      // Log error: Command output is empty
      logger.write(LogLevel::ERROR, "Error: Command output is empty");
      return "";
    }

    std::regex r(R"(mCurrentFocus=Window\{[^}]+ ([^/]+)/)");
    std::smatch match;
    if (std::regex_search(line, match, r))
    {
      return match[1].str();
    }
    else
    {
      // Log error: Unable to parse package name
      logger.write(LogLevel::ERROR, "Error: Unable to parse package name");
      return "";
    }
  }
};
